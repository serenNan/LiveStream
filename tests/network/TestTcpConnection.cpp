#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include <cstring>
#include <iostream>
#include <memory>

using namespace tmms::network;
// std::thread th;
// EventLoopThread eventloop_thread;

// // 定义 HTTP 响应
// const char *http_response =
//     "HTTP/1.0 200 OK\r\nServer: lss\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
// int main()
// {
//     eventloop_thread.Run();
//     EventLoop *loop = eventloop_thread.Loop();
//     // 如果事件循环对象有效
//     if (loop)
//     {
//         std::vector<TcpConnectionPtr> list;
//         InetAddress server("127.0.0.1:34444");
//         std::shared_ptr<Acceptor> acceptor = std::make_shared<Acceptor>(loop, server);

//         acceptor->SetAcceptCallback([&loop, &server, &list](int fd, const InetAddress &addr) {
//             std::cout << "host: " << addr.ToIpPort() << std::endl;
//             TcpConnectionPtr connection = std::make_shared<TcpConnection>(loop, fd, server, addr);
//             connection->SetRecMsgCallback([](const TcpConnectionPtr &con, MsgBuffer &buff) {
//                 std::cout << "recv msg: " << buff.Peek() << std::endl;
//                 buff.RetrieveAll();
//                 con->Send(http_response, strlen(http_response));
//             });

//             connection->SetWriteCompleteCallback([&loop](const TcpConnectionPtr &con) {
//                 std::cout << "write complete host: " << con->PeerAddr().ToIpPort() << std::endl;
//                 loop->DelEvent(con);
//                 con->ForceClose();
//             });
//             loop->AddEvent(connection);
//             list.push_back(connection);
//         });

//         acceptor->Start();

//         while (1)
//         {
//             std::this_thread::sleep_for(std::chrono::seconds(1));
//         }
//     }
//     return 0;
// }