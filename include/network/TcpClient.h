#pragma once
#include "network/base/InetAddress.h"
#include "network/net/EventLoop.h"
#include "network/net/TcpConnection.h"
#include <functional>

namespace tmms
{
    namespace network
    {
        enum
        {
            kTcpConStatusInit = 0,        ///< 初始化状态
            kTcpConStatusConnecting = 1,  ///< 连接中状态
            kTcpConStatusConnected = 2,   ///< 已连接状态
            kTcpConStatusDisConnected = 3 ///< 断开连接状态
        };

        using ConnectionCallback = std::function<void(const TcpConnectionPtr &con, bool)>;

        class TcpClient : public TcpConnection
        {
          public:
            TcpClient(EventLoop *loop, const InetAddress &server);
            void Connect();

            void SetConnectCallback(const ConnectionCallback &cb);

            void SetConnectCallback(ConnectionCallback &&cb);

            void OnRead() override;

            void OnWrite() override;

            void OnClose() override;

            void Send(std::list<BufferNodePtr> &list);

            void Send(const char *buff, size_t size);

            virtual ~TcpClient();

          private:
            void ConnectInLoop();

            void UpdateConnectionStatus();

            bool CheckError();
            InetAddress server_addr_;
            int32_t status_;
            ConnectionCallback connection_cb_;
            
        };
    } // namespace network
} // namespace tmms
