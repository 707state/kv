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
class RpcProvider {
public:
    // Notify the service to expose RPC methods
    void NotifyService(google::protobuf::Service* service);

    // Start RPC service node to provide remote calls
    void Run(int nodeIndex, short port);
    explicit RpcProvider(std::shared_ptr<boost::asio::io_context> context)
        : m_io_context(context) {}

private:
    // Replace Muduo's EventLoop with Boost.Asio's io_context
    std::shared_ptr<boost::asio::io_context> m_io_context;
    std::shared_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;

    struct ServiceInfo {
        google::protobuf::Service* m_service;
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*>
            m_methodMap;
    };

    // Store registered services and their method information
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

    // Callback for new connections
    void OnConnection(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket);
    // Callback for reading/writing events on an established connection
    void OnMessage(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket,
                   const boost::system::error_code& error,
                   std::size_t bytes_transferred);
    // Send RPC response
    void SendRpcResponse(const std::shared_ptr<boost::asio::ip::tcp::socket>& socket,
                         google::protobuf::Message* response);

public:
    ~RpcProvider() = default;
};
