#include "../friend.pb.h"
#include "boost/asio/io_context.hpp"
#include "rpc/mprpcchannel.h"
#include "rpc/rpcprovider.h"
#include <memory>
class FriendService : public fixbug::FiendServiceRpc {
public:
    std::vector<std::string> GetFriendsList(uint32_t userid) {
        std::cout << "local do GetFriendsList service! userid:" << userid << std::endl;
        std::vector<std::string> vec;
        vec.emplace_back("gao yang");
        vec.emplace_back("liu hong");
        vec.emplace_back("wang shuo");
        return vec;
    }

    // 重写基类方法
    void GetFriendsList(::google::protobuf::RpcController* controller,
                        const ::fixbug::GetFriendsListRequest* request,
                        ::fixbug::GetFriendsListResponse* response,
                        ::google::protobuf::Closure* done) {
        uint32_t userid = request->userid();
        std::vector<std::string> friendsList = GetFriendsList(userid);
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        for (std::string& name: friendsList) {
            std::string* p = response->add_friends();
            *p = name;
        }
        done->Run();
    }
};

int main(int argc, char** argv) {
    std::string ip = "127.0.0.1";
    short port = 7788;
    auto stub = new fixbug::FiendServiceRpc_Stub(new MprpcChannel(ip, port, false));
    // provider是一个rpc网络服务对象。把UserService对象发布到rpc节点上
    auto io_context = std::make_shared<boost::asio::io_context>(1);
    RpcProvider provider(io_context);
    provider.NotifyService(new FriendService());

    // 启动一个rpc服务发布节点   Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run(1, 7788);

    return 0;
}
