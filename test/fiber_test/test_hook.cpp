#include "fiber/iomanager.h"
#include "rpc/mprpcconfig.h"
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

const std::string LOG_HEAD = "[TASK] ";
void test_sleep() {
    std::cout << LOG_HEAD << "tid = " << ::GetThreadId() << ",test_fiber_sleep begin"
              << std::endl;
    ::IOManager iom(1, true);

    iom.scheduler([] {
        while (1) {
            sleep(6);
            std::cout << "task 1 sleep for 6s" << std::endl;
        }
    });

    iom.scheduler([] {
        while (1) {
            sleep(2);
            std::cout << "task2 sleep for 2s" << std::endl;
        }
    });

    std::cout << LOG_HEAD << "tid = " << ::GetThreadId() << ",test_fiber_sleep finish"
              << std::endl;
}
void test_socket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8000);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    std::cout << "begin connect" << std::endl;
    int rt = connect(sock, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
    std::cout << "connect rt=" << rt << " errno=" << errno << std::endl;

    if (rt) {
        return;
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    std::cout << "send rt=" << rt << " errno=" << errno << std::endl;

    if (rt <= 0) {
        return;
    }

    std::string buff;
    buff.resize(4096);

    rt = recv(sock, &buff[0], buff.size(), 0);
    std::cout << "recv rt=" << rt << " errno=" << errno << std::endl;

    if (rt <= 0) {
        return;
    }

    buff.resize(rt);
    std::cout << "--------------------------------" << std::endl;
    std::cout << buff << std::endl;
    std::cout << "--------------------------------" << std::endl;
}
int main() { test_sleep(); }
