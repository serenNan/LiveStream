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
        // TCP连接智能指针类型
        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
        // 关闭连接回调函数类型
        using CloseConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
        // 消息接收回调函数类型
        using MessageCallback = std::function<void(const TcpConnectionPtr &, MsgBuffer &buffer)>;
        // 写完成回调函数类型
        using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
        using TimeOutCallback = std::function<void(const TcpConnectionPtr &)>;

        /**
         * @brief TCP连接类，管理TCP连接的生命周期和事件处理
         * @details 负责处理TCP连接的读写事件、超时检测和关闭操作
         */
        class TcpConnection : public Connection
        {
          public:
            /**
             * @brief 构造函数
             * @param loop 事件循环指针
             * @param socketfd 套接字文件描述符
             * @param localAddr 本地地址
             * @param peerAddr 对端地址
             */
            TcpConnection(EventLoop *loop, int socketfd, const InetAddress &localAddr,
                          const InetAddress &peerAddr);

            ~TcpConnection();

            /**
             * @brief 设置关闭连接回调函数
             * @param cb 关闭连接回调函数
             */
            void SetCloseCallback(const CloseConnectionCallback &cb);
            /**
             * @brief 设置关闭连接回调函数（右值版本）
             * @param cb 关闭连接回调函数
             */
            void SetCloseCallback(CloseConnectionCallback &&cb);
            /**
             * @brief 设置接收消息回调函数
             * @param cb 接收消息回调函数
             */
            void SetRecvMsgCallback(const MessageCallback &cb);
            /**
             * @brief 设置接收消息回调函数（右值版本）
             * @param cb 接收消息回调函数
             */
            void SetRecvMsgCallback(MessageCallback &&cb);
            /**
             * @brief 设置写完成回调函数
             * @param cb 写完成回调函数
             */
            void SetWriteCompleteCallback(const WriteCompleteCallback &cb);
            /**
             * @brief 设置写完成回调函数（右值版本）
             * @param cb 写完成回调函数
             */
            void SetWriteCompleteCallback(WriteCompleteCallback &&cb);
            /**
             * @brief 设置超时回调函数
             * @param timeout 超时时间
             * @param cb 超时回调函数
             */
            void SetTimeoutCallback(int timeout, const TimeOutCallback &cb);
            /**
             * @brief 设置超时回调函数（右值版本）
             * @param timeout 超时时间
             * @param cb 超时回调函数
             */
            void SetTimeoutCallback(int timeout, TimeOutCallback &&cb);

            /**
             * @brief 发送数据
             * @param list 数据缓冲区列表
             */
            void Send(std::list<BufferNodePtr> &list);

            /**
             * @brief 发送数据
             * @param buf 数据缓冲区
             * @param size 数据大小
             */
            void Send(const void *buf, size_t size);

            /**
             * @brief 处理读事件
             */
            void OnRead() override;

            /**
             * @brief 处理写事件
             */
            void OnWrite() override;

            /**
             * @brief 处理关闭事件
             */
            void OnClose() override;

            /**
             * @brief 强制关闭连接
             */
            void ForceClose() override;

            /**
             * @brief 处理错误事件
             * @param msg 错误信息
             */
            void OnError(const std::string &msg) override;

            /**
             * @brief 处理超时事件
             */
            void OnTimeout();

            /**
             * @brief 启用空闲超时检测
             * @param max_time 最大空闲时间
             */
            void EnableCheckIdleTimeout(int32_t max_time);

          private:
            /**
             * @brief 在事件循环中发送数据
             * @param list 数据缓冲区列表
             */
            void SendInLoop(std::list<BufferNodePtr> &list);

            /**
             * @brief 在事件循环中发送数据
             * @param buf 数据缓冲区
             * @param size 数据大小
             */
            void SendInLoop(const void *buf, size_t size);

            /**
             * @brief 延长连接生命周期
             */
            void ExtendLife();

            bool close_{false};                         ///< 是否关闭连接
            CloseConnectionCallback close_cb_;          ///< 关闭连接回调函数
            MsgBuffer message_buffer_;                  ///< 消息缓冲区
            MessageCallback message_cb_;                ///< 消息回调函数
            std::vector<struct iovec> io_vec_list_;     ///< 向量列表，用于批量发送数据
            WriteCompleteCallback write_complete_cb_;   ///< 写完成回调函数
            std::weak_ptr<TimeOutEntry> timeout_entry_; ///< 超时事件条目
            int32_t max_idle_time_{30};                 ///< 最大空闲时间
        };
        struct TimeOutEntry
        {
          public:
            /**
             * @brief 构造函数
             * @param c TCP连接指针
             */
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

            std::weak_ptr<TcpConnection> conn; ///< TCP连接指针
        };
    } // namespace network
} // namespace tmms