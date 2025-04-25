#include "EventLoop.h"
#include "EventLoopThread.h"
#include "InetAddress.h"
#include "MsgBuffer.h"
#include "TcpConnection.h"
#include "TcpServer.h"
#include "TestContext.h"

#include <iostream>

using namespace tmms::network;
const char *http_response =
    "HTTP/1.0 200 OK\r\nServer: lss\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
using TextContextPtr = std::shared_ptr<TestContext>;

int main()
{
    EventLoopThread eventloop_thread;
    std::thread th;
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();
    if (loop)
    {
        InetAddress listen("127.0.0.1:34444");
        TcpServer server(loop, listen);
        server.SetMessageCallback([](const TcpConnectionPtr &con, MsgBuffer &buf) {
            TextContextPtr ctx = con->GetContext<TestContext>(kNormalContext);
            ctx->ParseMessage(buf);
            // std::cout << "host:" << con->PeerAddr().ToIpPort() << " meg:" << buf.Peek()
            //           << std::endl;
            // buf.RetrieveAll();
            //con->Send(http_response, strlen(http_response));
        });
        server.SetNewConnectionCallback([&loop](const TcpConnectionPtr &con) {
            TextContextPtr ctx = std::make_shared<TestContext>(con);
            ctx->SetTestMessageCallback(
                [](const TcpConnectionPtr &con, const std::string &msg) {
                    std::cout << "message: " << msg << std::endl;
                });
            con->SetContext(kNormalContext, ctx);
            con->SetWriteCompleteCallback([&loop](const TcpConnectionPtr &con) {
                std::cout << "write complete host: " << con->PeerAddr().ToIpPort() << std::endl;
                //con->ForceClose();
            });
        });
        server.Start();
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    return 0;
}