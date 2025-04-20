#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "InetAddress.h"

#include <iostream>
#include <memory>

using namespace tmms::network;

// int main()
// {
//     EventLoopThread eventloop_thread;
//     std::thread th;
//     eventloop_thread.Run();
//     EventLoop *loop = eventloop_thread.Loop();
//     if (loop)
//     {
//         InetAddress server("172.17.0.1:34444");
//         std::shared_ptr<Acceptor> acceptor = std::make_shared<Acceptor>(loop, server);
//         acceptor->setAcceptCallback([](int fd, const InetAddress &addr) {
//             std::cout << "连接成功 host:" << addr.ToIpPort() << std::endl;
//         });
//         acceptor->Start();
//         while (1)
//         {
//             std::this_thread::sleep_for(std::chrono::seconds(1));
//         }
//     }
//     return 0;
// }