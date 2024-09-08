#pragma once
#include "boost/asio.hpp"
#include "boost/asio/steady_timer.hpp"
#include "boost/asio/streambuf.hpp"
#include "boost/unordered_map.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/unordered/unordered_map_fwd.hpp>
#include <google/protobuf/service.h>
#include <memory>
#include <string>
// class RpcProvider {
// public:
//     RpcProvider(boost::asio::io_context& io_context)
//         : m_ioContext(io_context)
//         , m_acceptor(io_context)
//         , m_timer(io_context) {}
//     void NotifyService(google::protobuf::Service* service); // 发布rpc方法的函数接口
//     void Run(int nodeIndex, short port); // 启动rpc服务，开始提供rpc远程网络调用
//
// private:
//     boost::asio::io_context& m_ioContext;
//     boost::asio::ip::tcp::acceptor m_acceptor;
//     boost::asio::steady_timer m_timer;
//     struct ServiceInfo {
//         google::protobuf::Service* m_service;
//         boost::unordered_map<std::string, const google::protobuf::MethodDescriptor*>
//             m_mathodMap;
//     };
//     boost::unordered_map<std::string, ServiceInfo> m_serviceMap;
//     void StartAccept(); // 开始接受新的连接
//     void StartTimer();
//     void OnConnection(
//         std::shared_ptr<boost::asio::ip::tcp::socket> socket); // 新的socket连接回调
//     void OnMessage(
//         std::shared_ptr<boost::asio::ip::tcp::socket> socket,
//         std::shared_ptr<boost::asio::streambuf> buffer); //
//         已建立连接用户的读写事件回调
//     void
//     SendRpcResponse(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
//                     google::protobuf::Message*
//                         response); // Closure的回调操作，用于序列化rpc的响应和网络发送
//     void CheckInactiveConnections(const boost::system::error_code& ec) {
//         if (!ec) {
//             // TODO: 检查并关闭不活跃的连接
//             StartTimer(); // 重新启动定时器
//         }
//     }
//
// public:
//     ~RpcProvider();
// };
// RpcProvider 类定义
class RpcProvider {
public:
    RpcProvider(boost::asio::io_context& io_context)
        : io_context_(io_context)
        , acceptor_(io_context) {}
    void NotifyService(google::protobuf::Service* service);
    void Run(int nodeIndex, short port);
    ~RpcProvider();

    struct ServiceInfo {
        google::protobuf::Service* m_service;
        boost::unordered_map<std::string, const google::protobuf::MethodDescriptor*>
            m_mathodMap;
    };

private:
    void StartAccept();
    void OnConnection(const boost::system::error_code& ec,
                      std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void OnMessage(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void SendRpcResponse(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                         google::protobuf::Message* response);

    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;
};
