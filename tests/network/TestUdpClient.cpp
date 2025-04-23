#include "network/UdpClient.h"
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include <iostream>

using namespace tmms::network;

EventLoopThread eventloop_thread;

std::thread th;

int main(int argc, const char **agrv)
{
    eventloop_thread.Run();

    EventLoop *loop = eventloop_thread.Loop();

    if (loop)
    {
        InetAddress server("127.0.0.1:34444");

        std::shared_ptr<UdpClient> client = std::make_shared<UdpClient>(loop, server);

        client->SetRecvMsgCallback([](const InetAddress &addr, MsgBuffer &buff) {
            std::cout << "host: " << addr.ToIpPort() << " msg: " << buff.Peek() << std::endl;
            buff.RetrieveAll();
        });

        client->SetCloseCallback([](const UdpSocketPtr &con) {
            if (con)
            {
                std::cout << "host: " << con->PeerAddr().ToIpPort() << " closed." << std::endl;
            }
        });

        client->SetWriteCompleteCallback([](const UdpSocketPtr &con) {
            if (con)
            {
                std::cout << "host: " << con->PeerAddr().ToIpPort() << " write complete."
                          << std::endl;
            }
        });

        client->SetConnectedCallback([&client](const UdpSocketPtr &con, bool connected) {
            if (connected)
            {
                client->Send("11111", strlen("11111"));
            }
        });

        // 启动连接操作
        client->Connect();

        // 主线程进入无限循环，每隔一秒休眠一次
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    return 0;
}