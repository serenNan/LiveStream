#include "network/UdpServer.h"
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include <iostream>

using namespace tmms::network;

// 事件循环线程
EventLoopThread eventloop_thread;

// 标准线程
std::thread th;

int main(int argc, const char **agrv)
{
    // 启动事件循环线程
    eventloop_thread.Run();

    // 获取事件循环对象
    EventLoop *loop = eventloop_thread.Loop();

    if (loop)
    {
        // 初始化服务器监听地址
        InetAddress listen("127.0.0.1:34444");
        // 创建UDP服务器实例
        std::shared_ptr<UdpServer> server = std::make_shared<UdpServer>(loop, listen);

        // 设置消息接收回调
        server->SetRecvMsgCallback([&server](const InetAddress &addr, MsgBuffer &buff) {
            std::cout << "host: " << addr.ToIpPort() << " msg: " << buff.Peek() << std::endl;

            struct sockaddr_in6 sock_addr;
            addr.GetSockAddr((struct sockaddr *)&sock_addr);

            // 回显收到的数据
            server->Send(buff.Peek(), buff.ReadableBytes(), (struct sockaddr *)&sock_addr,
                         sizeof(sock_addr));
            buff.RetrieveAll();
        });

        // 设置连接关闭回调
        server->SetCloseCallback([](const UdpSocketPtr &con) {
            if (con)
            {
                std::cout << "host: " << con->PeerAddr().ToIpPort() << " closed." << std::endl;
            }
        });

        // 设置写操作完成回调
        server->SetWriteCompleteCallback([](const UdpSocketPtr &con) {
            if (con)
            {
                std::cout << "host: " << con->PeerAddr().ToIpPort() << " write complete. "
                          << std::endl;
            }
        });

        // 启动服务器
        server->Start();

        // 主循环
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    return 0;
}
