#include "EventLoop.h"
#include "EventLoopThread.h"
#include "InetAddress.h"
#include "MsgBuffer.h"
#include "TcpConnection.h"
#include "TcpServer.h"

#include <iostream>

using namespace tmms::network;
const char *http_response =
    "HTTP/1.0 200 OK\r\nServer: lss\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";

// int main()
// {
//     EventLoopThread eventloop_thread;
//     std::thread th;
//     eventloop_thread.Run();
//     EventLoop *loop = eventloop_thread.Loop();
//     if (loop)
//     {
//         InetAddress listen("172.17.0.1:34444");
//         TcpServer server(loop, listen);
//         server.SetMessageCallback([](const TcpConnectionPtr &con, MsgBuffer &buf) {
//             std::cout << "host:" << con->PeerAddr().ToIpPort() << " meg:" << buf.Peek()
//                       << std::endl;
//             buf.RetrieveAll();
//             con->Send(http_response, strlen(http_response));
//         });
//         server.SetNewConnectionCallback([&loop](const TcpConnectionPtr &con) {
//             con->SetWriteCompleteCallback([&loop](const TcpConnectionPtr &con) {
//                 std::cout << "write complete host: " << con->PeerAddr().ToIpPort() << std::endl;
//                 con->ForceClose();
//             });
//         });
//         server.Start();
//         while (1)
//         {
//             std::this_thread::sleep_for(std::chrono::seconds(1));
//         }
//     }
//     return 0;
// }