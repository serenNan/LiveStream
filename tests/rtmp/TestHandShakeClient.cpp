#include "mmedia/rtmp/RtmpHandShake.h"
#include "network/TcpClient.h"
#include "network/net/Acceptor.h"
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include <iostream>

using namespace tmms::network;
using namespace tmms::mm;

// RTMP握手流程说明：
// 1. C0: 客户端发送版本号(1字节)
// 2. S0: 服务器返回版本号(1字节)
// 3. C1: 客户端发送时间戳(4字节)和随机数据(1528字节)
// 4. S1: 服务器返回时间戳(4字节)和随机数据(1528字节)
// 5. C2: 客户端发送S1的副本
// 6. S2: 服务器发送C1的副本
// 握手完成后，开始RTMP消息交互

// 声明一个EventLoopThread对象，用于管理事件循环的线程
EventLoopThread eventloop_thread;
// 声明一个标准线程对象th
std::thread th;

// 定义一个智能指针类型RtmpHandShakePtr，用于管理RtmpHandShake对象的共享所有权
using RtmpHandShakePtr = std::shared_ptr<RtmpHandShake>;

// 定义两个字符串常量，http_request和http_response，分别表示一个简单的HTTP GET请求和HTTP响应
const char *http_request = "GET / HTTP/1.0\r\nHost: 192.168.56.168\r\nAccept: */*\r\nContent-Type: "
                           "text/plain\r\nContent-Length: 0\r\n\r\n";
const char *http_response =
    "HTTP/1.0 200 OK\r\nServer: lss\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";

int main(int argc, const char **agrv)
{
    // 启动事件循环线程，开始处理事件循环
    eventloop_thread.Run();

    // 获取事件循环对象指针，用于后续操作
    EventLoop *loop = eventloop_thread.Loop();

    if (loop)
    {
        // 创建一个InetAddress对象，用于存储服务器地址信息，IP地址为192.168.56.168，端口号为1935（RTMP协议默认端口）
        InetAddress server("127.0.0.1:1935");

        // 创建一个TcpClient智能指针对象，表示一个TCP客户端，并将其与事件循环和服务器地址绑定
        std::shared_ptr<TcpClient> client = std::make_shared<TcpClient>(loop, server);

        // 设置TCP客户端的接收消息回调函数，用于处理RTMP握手过程中的数据交换
        client->SetRecvMsgCallback([](const TcpConnectionPtr &con, MsgBuffer &buff) {
            // 当接收到消息时，从连接对象的上下文中获取RtmpHandShake对象，并调用其HandShake方法处理数据
            // HandShake方法会根据当前握手阶段处理C0/C1/C2或S0/S1/S2数据
            RtmpHandShakePtr shake = con->GetContext<RtmpHandShake>(kNormalContext);
            shake->HandShake(buff);
        });

        // 设置TCP客户端的关闭连接回调函数
        client->SetCloseCallback([](const TcpConnectionPtr &con) {
            if (con)
            {
                // 当连接关闭时，输出连接的对端IP地址和端口信息
                std::cout << "host: " << con->PeerAddr().ToIpPort() << " closed." << std::endl;
            }
        });

        // 设置TCP客户端的写入完成回调函数，用于处理握手数据发送完成后的逻辑
        client->SetWriteCompleteCallback([](const TcpConnectionPtr &con) {
            if (con)
            {
                // 当数据写入完成时，输出连接的对端IP地址和端口信息，并调用RtmpHandShake对象的WriteComplete方法
                // WriteComplete方法会处理握手状态转换，例如从C0发送完成转换到等待S0等
                std::cout << "host: " << con->PeerAddr().ToIpPort() << " write complete. "
                          << std::endl;
                RtmpHandShakePtr shake = con->GetContext<RtmpHandShake>(kNormalContext);
                shake->WriteComplete();
            }
        });

        // 设置TCP客户端的连接回调函数，在TCP连接建立后初始化RTMP握手
        client->SetConnectCallback([](const TcpConnectionPtr &con, bool connected) {
            if (connected)
            {
                // 当连接成功时，创建RtmpHandShake对象，并将其与连接关联
                // 第二个参数true表示这是客户端握手
                RtmpHandShakePtr shake = std::make_shared<RtmpHandShake>(con, true);
                con->SetContext(kNormalContext, shake);
                // 调用Start方法开始握手过程，发送C0数据
                shake->Start();
            }
        });

        // 发起TCP连接请求
        client->Connect();

        // 主线程进入一个无限循环，每次休眠1秒，保持程序运行
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    return 0;
}
