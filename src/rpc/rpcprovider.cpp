#include "boost/asio/ip/address.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/system/detail/error_code.hpp"
#include "rpc/rpcheader.pb.h"
#include <arpa/inet.h>
#include <cstdlib>
#include <fstream>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <rpc/rpcprovider.h>
#include <string>
#include <unistd.h>
using namespace boost::asio::ip;
void RpcProvider::NotifyService(google::protobuf::Service* service) {
    ServiceInfo service_info;
    const google::protobuf::ServiceDescriptor* pserviceDesc = service->GetDescriptor();
    std::string service_name = pserviceDesc->name();
    int methodCnt = pserviceDesc->method_count();
    std::cout << "service_name: " << service_name << std::endl;

    for (int i = 0; i < methodCnt; ++i) {
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_mathodMap.insert({method_name, pmethodDesc});
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

void RpcProvider::Run(int nodeIndex, short port) {
    // 获取IP地址
    char* ipC = nullptr;
    char hname[128];
    struct hostent* hent;
    gethostname(hname, sizeof(hname));
    hent = gethostbyname(hname);
    for (int i = 0; hent->h_addr_list[i]; i++) {
        ipC = inet_ntoa(*reinterpret_cast<struct in_addr*>(hent->h_addr_list[i]));
    }
    std::string ip = std::string(ipC);

    // 写入配置文件
    std::string node = "node" + std::to_string(nodeIndex);
    std::ofstream outfile;
    outfile.open("test.conf", std::ios::app);
    if (!outfile.is_open()) {
        std::cout << "打开文件失败！" << std::endl;
        exit(EXIT_FAILURE);
    }
    outfile << node + "ip=" + ip << std::endl;
    outfile << node + "port=" + std::to_string(port) << std::endl;
    outfile.close();

    // 初始化 acceptor
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(ip), port);
    acceptor_ = boost::asio::ip::tcp::acceptor(io_context_, endpoint);

    // 开始监听连接
    StartAccept();

    std::cout << "RpcProvider start service at ip: " << ip << " port: " << port
              << std::endl;

    // 运行 io_context 事件循环
    io_context_.run();
}

void RpcProvider::StartAccept() {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
    acceptor_.async_accept(
        *socket,
        std::bind(
            &RpcProvider::OnConnection, this, boost::asio::placeholders::error, socket));
}

void RpcProvider::OnConnection(const boost::system::error_code& ec,
                               std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    if (!ec) {
        // 新连接已建立，启动读取消息流程
        OnMessage(socket);
    }
    // 继续等待下一个连接
    StartAccept();
}

void RpcProvider::OnMessage(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    auto buffer = std::make_shared<std::vector<char>>(1024); // 读缓存
    boost::asio::async_read(
        *socket,
        boost::asio::buffer(*buffer),
        [this, socket, buffer](const boost::system::error_code& ec,
                               std::size_t bytes_transferred) {
            if (!ec) {
                std::string recv_buf(buffer->data(), bytes_transferred);

                // 解析收到的 Protobuf 消息
                google::protobuf::io::ArrayInputStream array_input(recv_buf.data(),
                                                                   recv_buf.size());
                google::protobuf::io::CodedInputStream coded_input(&array_input);
                uint32_t header_size{};
                coded_input.ReadVarint32(&header_size);

                // 根据header_size读取数据头的原始字符流，反序列化数据
                std::string rpc_header_str;
                RPC::RpcHeader rpcHeader;
                std::string service_name;
                std::string method_name;

                google::protobuf::io::CodedInputStream::Limit msg_limit =
                    coded_input.PushLimit(header_size);
                coded_input.ReadString(&rpc_header_str, header_size);
                coded_input.PopLimit(msg_limit);

                uint32_t args_size{};
                if (rpcHeader.ParseFromString(rpc_header_str)) {
                    service_name = rpcHeader.service_name();
                    method_name = rpcHeader.method_name();
                    args_size = rpcHeader.args_size();
                } else {
                    std::cout << "rpc_header_str:" << rpc_header_str << " parse error!"
                              << std::endl;
                    return;
                }

                std::string args_str;
                bool read_args_success = coded_input.ReadString(&args_str, args_size);
                if (!read_args_success) {
                    return;
                }

                auto it = m_serviceMap.find(service_name);
                if (it == m_serviceMap.end()) {
                    std::cout << "服务：" << service_name << " is not exist!"
                              << std::endl;
                    return;
                }
                auto mit = it->second.m_mathodMap.find(method_name);
                if (mit == it->second.m_mathodMap.end()) {
                    std::cout << service_name << ":" << method_name << " is not exist!"
                              << std::endl;
                    return;
                }
                google::protobuf::Service* service = it->second.m_service;
                const google::protobuf::MethodDescriptor* method = mit->second;

                google::protobuf::Message* request =
                    service->GetRequestPrototype(method).New();
                if (!request->ParseFromString(args_str)) {
                    std::cout << "request parse error, content:" << args_str << std::endl;
                    return;
                }
                google::protobuf::Message* response =
                    service->GetResponsePrototype(method).New();

                google::protobuf::Closure* done = google::protobuf::NewCallback<
                    RpcProvider,
                    std::shared_ptr<boost::asio::ip::tcp::socket>,
                    google::protobuf::Message*>(
                    this, &RpcProvider::SendRpcResponse, socket, response);

                service->CallMethod(method, nullptr, request, response, done);
            }
        });
}

void RpcProvider::SendRpcResponse(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                                  google::protobuf::Message* response) {
    std::string response_str;
    if (response->SerializeToString(&response_str)) {
        boost::asio::async_write(*socket,
                                 boost::asio::buffer(response_str),
                                 [this, socket](const boost::system::error_code& ec,
                                                std::size_t /*bytes_transferred*/) {
                                     if (ec) {
                                         std::cout << "serialize response_str error!"
                                                   << std::endl;
                                     }
                                 });
    }
}

RpcProvider::~RpcProvider() { io_context_.stop(); }
