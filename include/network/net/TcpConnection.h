#pragma once

#include "Connection.h"
#include "InetAddress.h"
#include "MsgBuffer.h"
#include <cstddef>
#include <functional>
#include <list>
#include <memory>
#include <sys/uio.h>
#include <vector>

namespace tmms
{
    namespace network
    {
        class TcpConnection;
        struct TimeOutEntry;
        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
        using CloseConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
        using MessageCallback = std::function<void(const TcpConnectionPtr &, MsgBuffer &buffer)>;
        using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
        using TimeOutCallback = std::function<void(const TcpConnectionPtr &)>;
        struct BufferNode
        {
            BufferNode(void *buf, size_t s) : addr(buf), size(s)
            {
            }
            void *addr;
            size_t size{0};
        };
        using BufferNodePtr = std::shared_ptr<BufferNode>;

        class TcpConnection : public Connection
        {
          public:
            TcpConnection(EventLoop *loop, int socketfd, const InetAddress &localAddr,
                          const InetAddress &peerAddr);
            ~TcpConnection();

            void SetCloseCallback(const CloseConnectionCallback &cb);
            void SetCloseCallback(CloseConnectionCallback &&cb);
            void SetRecMsgCallback(const MessageCallback &cb);
            void SetRecMsgCallback(MessageCallback &&cb);
            void SetWriteCompleteCallback(const WriteCompleteCallback &cb);
            void SetWriteCompleteCallback(WriteCompleteCallback &&cb);
            void SetTimeoutCallback(int timeout, const TimeOutCallback &cb);
            void SetTimeoutCallback(int timeout, TimeOutCallback &&cb);

            void Send(std::list<BufferNodePtr> &list);
            void Send(const void *buf, size_t size);

            void OnRead() override;
            void OnWrite() override;
            void OnClose() override;
            void ForceClose() override;
            void OnError(const std::string &msg) override;

            void OnTimeout();
            void EnableCheckIdleTimeout(int32_t max_time);

          private:
            void SendInLoop(std::list<BufferNodePtr> &list);
            void SendInLoop(const void *buf, size_t size);
            void ExtendLife();

            bool close_{false};
            CloseConnectionCallback close_cb_;
            MsgBuffer message_buffer_;
            MessageCallback message_cb_;
            std::vector<struct iovec> io_vec_list_;
            WriteCompleteCallback write_complete_cb_;
            std::weak_ptr<TimeOutEntry> timeout_entry_;
            int32_t max_idle_time_{30};
        };
        struct TimeOutEntry
        {
          public:
            TimeOutEntry(const TcpConnectionPtr &c)
                : conn(c){

                  };
            ~TimeOutEntry()
            {
                auto c = conn.lock();
                if (c)
                {
                    c->OnTimeout();
                }
            };
            std::weak_ptr<TcpConnection> conn;
        };
    } // namespace network
} // namespace tmms