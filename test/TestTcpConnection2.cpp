#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include "gtest/gtest.h"
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <thread>

using namespace tmms::network;

class TcpConnectionTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        eventloop_thread.Run();
        loop = eventloop_thread.Loop();
        server = InetAddress("127.0.0.1:34444");
        acceptor = std::make_shared<Acceptor>(loop, server);
    }

    void TearDown() override
    {
        connections.clear();
    }

    EventLoopThread eventloop_thread;
    EventLoop *loop;
    InetAddress server;
    std::shared_ptr<Acceptor> acceptor;
    std::vector<TcpConnectionPtr> connections;
};

TEST_F(TcpConnectionTest, BasicConnection)
{
    acceptor->SetAcceptCallback([this](int fd, const InetAddress &addr) {
        TcpConnectionPtr conn = std::make_shared<TcpConnection>(loop, fd, server, addr);

        conn->SetRecMsgCallback([](const TcpConnectionPtr &conn, MsgBuffer &buffer) {
            std::cout << "Received message, readable bytes: " << buffer.ReadableBytes()
                      << std::endl;
            std::string msg(buffer.Peek(), buffer.ReadableBytes());
            EXPECT_FALSE(msg.empty());
            std::cout << "Message content: " << msg << std::endl;
            buffer.RetrieveAll();
            conn->Send("OK", 2);
        });

        conn->SetCloseCallback([this](const TcpConnectionPtr &conn) {
            connections.erase(std::remove(connections.begin(), connections.end(), conn),
                              connections.end());
        });

        connections.push_back(conn);
        loop->AddEvent(conn);
    });

    acceptor->Start();

    // Simulate client connection and data sending
    std::thread client_thread([this]() {
        std::cout << "Client thread started" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr;
        server.GetSockAddr((struct sockaddr *)&addr);
        if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            std::cerr << "Connect failed: " << strerror(errno) << std::endl;
            close(sockfd);
            return;
        }

        const char *msg = "Test message";
        std::cout << "Sending message: " << msg << std::endl;
        if (write(sockfd, msg, strlen(msg)) < 0)
        {
            std::cerr << "Write failed: " << strerror(errno) << std::endl;
            close(sockfd);
            return;
        }

        char buf[1024];
        ssize_t n = read(sockfd, buf, sizeof(buf));
        if (n <= 0)
        {
            std::cerr << "Read failed: " << (n == 0 ? "connection closed" : strerror(errno))
                      << std::endl;
            close(sockfd);
            return;
        }
        std::cout << "Received response: " << std::string(buf, n) << std::endl;
        close(sockfd);
    });

    client_thread.join();

    EXPECT_EQ(connections.size(), 1);
}

TEST_F(TcpConnectionTest, TimeoutTest)
{
    acceptor->SetAcceptCallback([this](int fd, const InetAddress &addr) {
        TcpConnectionPtr conn = std::make_shared<TcpConnection>(loop, fd, server, addr);

        conn->EnableCheckIdleTimeout(1); // 1 second timeout

        conn->SetTimeoutCallback(30, [](const TcpConnectionPtr &conn) { conn->ForceClose(); });

        connections.push_back(conn);
        loop->AddEvent(conn);
    });

    acceptor->Start();

    std::thread client_thread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr;
        server.GetSockAddr((struct sockaddr *)&addr);
        connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));

        std::this_thread::sleep_for(std::chrono::seconds(2));
        close(sockfd);
    });

    client_thread.join();

    EXPECT_TRUE(connections.empty());
}

// int main(int argc, char** argv) {
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }