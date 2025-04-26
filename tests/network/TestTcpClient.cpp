#include "EventLoop.h"
#include "EventLoopThread.h"
#include "InetAddress.h"
#include "MsgBuffer.h"
#include "TcpClient.h"
#include "TcpConnection.h"

#include <iostream>

using namespace tmms::network;
// 定义HTTP请求字符串
const char *http_request =
    "GET / HTTP/1.0\r\nHost: 172.17.0.1:34444\r\nAccept: */*\r\nContent-Type: "
    "text/plain\r\nContent-Length: 0\r\n\r\n";
// 定义HTTP响应字符串
const char *http_response =
    "HTTP/1.0 200 OK\r\nServer: lss\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";

int main()
{
    EventLoopThread eventloop_thread;
    std::thread th;
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();
    if (loop)
    {
        InetAddress server("127.0.0.1:34444");
        std::shared_ptr<TcpClient> client = std::make_shared<TcpClient>(loop, server);
        client->SetRecvMsgCallback([](const TcpConnectionPtr &con, MsgBuffer &buf) {
            std::cout << "host:" << con->PeerAddr().ToIpPort() << " meg:" << buf.Peek()
                      << std::endl;
            buf.RetrieveAll();
            con->Send(http_response, strlen(http_response));
        });
        client->SetCloseCallback([](const TcpConnectionPtr &con) {
            if (con)
            {
                std::cout << "host: " << con->PeerAddr().ToIpPort() << " closed." << std::endl;
            }
        });
        client->SetWriteCompleteCallback([](const TcpConnectionPtr &con) {
            if (con)
            {
                std::cout << "host: " << con->PeerAddr().ToIpPort() << " write complete"
                          << std::endl;
            }
        });
        client->SetConnectCallback([](const TcpConnectionPtr &con, bool connected) {
            if (connected)
            {
                auto size = htonl(strlen(http_request));
                con->Send((const char *)&size, sizeof(size));
                con->Send(http_request, strlen(http_request));
            }
        });
        client->Connect();
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    return 0;
}