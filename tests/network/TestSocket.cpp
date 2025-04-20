#include "InetAddress.h"
#include "SocketOpt.h"
#include <cerrno>
#include <iostream>

using namespace tmms::network;

void TestClient()
{
    int sock = SocketOpt::CreateNonBlockingTcpSocket(AF_INET);
    if (sock < 0)
    {
        std::cout << "socket failed.sock:" << sock << std::endl;
        return;
    }
    InetAddress server("127.0.0.1:34444");
    SocketOpt opt(sock);
    opt.SetNonBlocking(false);
    auto ret = opt.Connect(server);
    if (ret < 0)
    {
        std::cout << "connect failed.error:" << errno << std::endl;
        return;
    }
    std::cout << "connect success.ret:" << ret << std::endl
              << " local:" << opt.GetLocalAddress()->ToIpPort() << std::endl
              << " peer:" << opt.GetPeerAddress()->ToIpPort() << std::endl;
}

void TestServer()
{
    int sock = SocketOpt::CreateNonBlockingTcpSocket(AF_INET);
    if (sock < 0)
    {
        std::cout << "socket failed.sock:" << sock << std::endl;
        return;
    }
    InetAddress server("0.0.0.1:34444");
    SocketOpt opt(sock);
    opt.SetNonBlocking(false);
    opt.BindAddress(server);
    opt.Listen();
    InetAddress addr;
    auto ns = opt.Accept(&addr);
    std::cout << "Accept success.ret:" << ns << std::endl
              << " addr:" << addr.ToIpPort() << std::endl;
}

// int main()
// {
//     TestServer();
//     return 0;
// }