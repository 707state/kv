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
