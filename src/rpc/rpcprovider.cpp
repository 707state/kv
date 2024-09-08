#include "boost/asio/ip/address.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/system/detail/error_code.hpp"
#include "rpc/rpcheader.pb.h"
#include <arpa/inet.h>
#include <cstdlib>
#include <fstream>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
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
        ipC = inet_ntoa(*(struct in_addr*)(hent->h_addr_list[i]));
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
// void RpcProvider::NotifyService(google::protobuf::Service* service) {
//     ServiceInfo service_info;
//     const google::protobuf::ServiceDescriptor* pserviceDesc = service->GetDescriptor();
//     auto service_name = pserviceDesc->name();
//     int methodCnt = pserviceDesc->method_count();
//     std::cout << "service_name:" << service_name << std::endl;
//     for (int i = 0; i < methodCnt; i++) {
//         // 获取了服务对象指定下标的服务方法的描述（抽象描述） UserService   Login
//         const google::protobuf::MethodDescriptor* pmethodDesc =
//         pserviceDesc->method(i); std::string method_name = pmethodDesc->name();
//         service_info.m_mathodMap.insert({method_name, pmethodDesc});
//     }
//     service_info.m_service = service;
//     m_serviceMap.insert({service_name, service_info});
// }
// void RpcProvider::Run(int nodeIndex, short port) {
//     char* ipC;
//     char hname[128];
//     struct hostent* hent;
//     gethostname(hname, sizeof(hname));
//     hent = gethostbyname(hname);
//     for (int i = 0; hent->h_addr_list[i]; i++) {
//         ipC = inet_ntoa(*reinterpret_cast<struct in_addr*>(hent->h_addr_list[i]));
//     }
//     std::string ip = std::string(ipC);
//     std::string node = "node" + std::to_string(nodeIndex);
//     std::ofstream outfile;
//     outfile.open("test.conf", std::ios::app);
//     if (!outfile.is_open()) {
//         std::cout << "打开文件失败!" << std::endl;
//         exit(EXIT_FAILURE);
//     }
//     outfile << node + "ip=" + ip << std::endl;
//     outfile << node + "port=" + std::to_string(port) << std::endl;
//     outfile.close();
//     tcp::endpoint endpoint(boost::asio::ip::make_address(ip), port);
//     m_acceptor.open(endpoint.protocol());
//     m_acceptor.bind(endpoint);
//     m_acceptor.listen();
//     std::cout << "RpcProvider start service at ip:" << ip << " port:" << port
//               << std::endl;
//     std::shared_ptr<tcp::socket> socket = std::make_shared<tcp::socket>(m_ioContext);
//     StartAccept();
//     StartTimer();
//     m_ioContext.run();
// }
// void RpcProvider::OnConnection(std::shared_ptr<tcp::socket> socket) {
//     if (socket && socket->is_open()) {
//         // 正常接收，什么都不做
//     } else {
//         boost::system::error_code ec;
//         socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
//         if (ec) {
//             std::cerr << "Error during socket shutdown: " << ec.message() << std::endl;
//         }
//         socket->close();
//     }
// }
// void RpcProvider::OnMessage(std::shared_ptr<tcp::socket> socket,
//                             std::shared_ptr<boost::asio::streambuf> buffer) {
//     std::istream is(buffer.get());
//     std::string recv_buf((std::istreambuf_iterator<char>(is)),
//                          std::istreambuf_iterator<char>());
//     // 使用 protobuf 的 CodedInputStream 来解析数据流
//     google::protobuf::io::ArrayInputStream array_input(recv_buf.data(),
//     recv_buf.size()); google::protobuf::io::CodedInputStream coded_input(&array_input);
//     uint32_t header_size{};
//     coded_input.ReadVarint32(&header_size); // 解析 header_size
//     // 根据 header_size 读取数据头的原始字符流，反序列化数据，得到 RPC 请求的详细信息
//     std::string rpc_header_str;
//     RPC::RpcHeader rpcHeader;
//     std::string service_name;
//     std::string method_name;
//     // 设置读取限制，不必担心数据读多
//     google::protobuf::io::CodedInputStream::Limit msg_limit =
//         coded_input.PushLimit(header_size);
//     coded_input.ReadString(&rpc_header_str, header_size);
//     // 恢复之前的限制，以便安全地继续读取其他数据
//     coded_input.PopLimit(msg_limit);
//     uint32_t args_size{};
//     if (rpcHeader.ParseFromString(rpc_header_str)) {
//         // 数据头反序列化成功
//         service_name = rpcHeader.service_name();
//         method_name = rpcHeader.method_name();
//         args_size = rpcHeader.args_size();
//     } else {
//         // 数据头反序列化失败
//         std::cout << "rpc_header_str:" << rpc_header_str << " parse error!" <<
//         std::endl; return;
//     }
//     // 获取 RPC 方法参数的字符流数据
//     std::string args_str;
//     bool read_args_success = coded_input.ReadString(&args_str, args_size);
//
//     if (!read_args_success) {
//         // 处理错误：参数数据读取失败
//         return;
//     }
//     // 获取 service 对象和 method 对象
//     auto it = m_serviceMap.find(service_name);
//     if (it == m_serviceMap.end()) {
//         std::cout << "服务：" << service_name << " is not exist!" << std::endl;
//         std::cout << "当前已经有的服务列表为:";
//         for (const auto& item: m_serviceMap) {
//             std::cout << item.first << " ";
//         }
//         std::cout << std::endl;
//         return;
//     }
//     auto mit = it->second.m_mathodMap.find(method_name);
//     if (mit == it->second.m_mathodMap.end()) {
//         std::cout << service_name << ":" << method_name << " is not exist!" <<
//         std::endl; return;
//     }
//     google::protobuf::Service* service = it->second.m_service;
//     const google::protobuf::MethodDescriptor* method = mit->second;
//
//     // 生成 RPC 方法调用的 request 和 response 参数
//     google::protobuf::Message* request = service->GetRequestPrototype(method).New();
//     if (!request->ParseFromString(args_str)) {
//         std::cout << "request parse error, content:" << args_str << std::endl;
//         return;
//     }
//     google::protobuf::Message* response = service->GetResponsePrototype(method).New();
//
//     // 定义 done 回调，用于处理响应
//     // FIXME: 这个地方的模板参数可能是错的
//     google::protobuf::Closure* done =
//         google::protobuf::NewCallback<RpcProvider,
//                                       std::shared_ptr<boost::asio::ip::tcp::socket>,
//                                       google::protobuf::Message*>(
//             this, &RpcProvider::SendRpcResponse, socket, response);
//     // 调用方法
//     service->CallMethod(method, nullptr, request, response, done);
// }
// void RpcProvider::StartTimer() {
//     m_timer.expires_after(std::chrono::minutes(5)); // 每5分钟检查一次
//     m_timer.async_wait(
//         [this](const boost::system::error_code& ec) { CheckInactiveConnections(ec); });
// }
// void RpcProvider::SendRpcResponse(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
//                                   google::protobuf::Message* response) {
//     std::string response_str;
//
//     // Attempt to serialize the response to a string
//     if (!response->SerializeToString(&response_str)) {
//         std::cerr << "Error: Failed to serialize response!" << std::endl;
//         return;
//     }
//
//     // Prepare the data to be sent over the network
//     auto buffer = std::make_shared<std::string>(std::move(response_str));
//
//     // Asynchronously write the serialized response to the socket
//     boost::asio::async_write(
//         *socket,
//         boost::asio::buffer(*buffer),
//         [socket, buffer](const boost::system::error_code& ec,
//                          std::size_t bytes_transferred) {
//             if (ec) {
//                 std::cerr << "Error: Failed to send response - " << ec.message()
//                           << std::endl;
//             } else {
//                 std::cout << "Response sent successfully, bytes transferred: "
//                           << bytes_transferred << std::endl;
//             }
//             // Long connection: Do not close the socket
//         });
// }
// void RpcProvider::StartAccept() {
//     auto socket = std::make_shared<boost::asio::ip::tcp::socket>(m_ioContext);
//     m_acceptor.async_accept(
//         *socket,
//         std::bind(
//             &RpcProvider::OnConnection, this, boost::asio::placeholders::error,
//             socket));
// }
// RpcProvider::~RpcProvider() { m_ioContext.stop(); }
