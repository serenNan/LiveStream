#pragma once
#include "TcpConnection.h"
#include "network/base/InetAddress.h"
#include "network/base/MsgBuffer.h"
#include "network/net/Connection.h"
#include "network/net/EventLoop.h"
#include <functional>
#include <list>
#include <memory>

namespace tmms
{
    namespace network
    {
        class UdpSocket;
        /// @brief UDP套接字智能指针类型
        using UdpSocketPtr = std::shared_ptr<UdpSocket>;
        struct UdpTimeoutEntry;

        /**
         * @brief UDP缓冲区节点结构体
         * 继承自BufferNode，增加UDP特有的地址信息
         */
        struct UdpBufferNode : public BufferNode
        {
            /**
             * @brief 构造函数
             * @param buff 数据缓冲区指针
             * @param s 数据大小
             * @param saddr 目标地址结构体指针
             * @param len 地址结构体长度
             */
            UdpBufferNode(void *buff, size_t s, struct sockaddr *saddr, socklen_t len)
                : BufferNode(buff, s), sock_addr(saddr), sock_len(len)
            {
            }
            struct sockaddr *sock_addr{nullptr}; ///< 目标地址结构体指针
            socklen_t sock_len{0};               ///< 地址结构体长度
        };

        /// @brief UDP缓冲区节点智能指针类型
        using UdpBufferNodePtr = std::shared_ptr<UdpBufferNode>;

        /// @brief UDP消息接收回调函数类型
        using UdpSocketMessageCallback =
            std::function<void(const InetAddress &addr, MsgBuffer &buff)>;

        /// @brief UDP写完成回调函数类型
        using UdpSocketWriteCompleteCallback = std::function<void(const UdpSocketPtr &)>;

        /// @brief UDP连接关闭回调函数类型
        using UdpSocketCloseConnectionCallback = std::function<void(const UdpSocketPtr &)>;

        /// @brief UDP超时回调函数类型
        using UdpSocketTimeoutCallback = std::function<void(const UdpSocketPtr &)>;

        /**
         * @brief UDP套接字类
         * 继承自Connection，实现UDP协议的网络通信功能
         */
        class UdpSocket : public Connection
        {
          public:
            /**
             * @brief 构造函数
             * @param loop 所属事件循环
             * @param socketfd 套接字文件描述符
             * @param localAddr 本地地址
             * @param peerAddr 对端地址
             */
            UdpSocket(EventLoop *loop, int socketfd, const InetAddress &localAddr,
                      const InetAddress &peerAddr);
            ~UdpSocket();

            /**
             * @brief 设置连接关闭回调函数
             * @param cb 回调函数，当连接关闭时触发
             */
            void SetCloseCallback(const UdpSocketCloseConnectionCallback &cb);

            /**
             * @brief 设置连接关闭回调函数(移动语义)
             * @param cb 回调函数，当连接关闭时触发
             */
            void SetCloseCallback(UdpSocketCloseConnectionCallback &&cb);

            /**
             * @brief 设置消息接收回调函数
             * @param cb 回调函数，当收到消息时触发
             */
            void SetRecvMsgCallback(const UdpSocketMessageCallback &cb);

            /**
             * @brief 设置消息接收回调函数(移动语义)
             * @param cb 回调函数，当收到消息时触发
             */
            void SetRecvMsgCallback(UdpSocketMessageCallback &&cb);

            /**
             * @brief 设置写完成回调函数
             * @param cb 回调函数，当数据写入完成时触发
             */
            void SetWriteCompleteCallback(const UdpSocketWriteCompleteCallback &cb);

            /**
             * @brief 设置写完成回调函数(移动语义)
             * @param cb 回调函数，当数据写入完成时触发
             */
            void SetWriteCompleteCallback(UdpSocketWriteCompleteCallback &&cb);

            /**
             * @brief 设置超时回调函数
             * @param timeout 超时时间(秒)
             * @param cb 回调函数，当超时发生时触发
             */
            void SetTimeoutCallback(int timeout, const UdpSocketTimeoutCallback &cb);

            /**
             * @brief 设置超时回调函数(移动语义)
             * @param timeout 超时时间(秒)
             * @param cb 回调函数，当超时发生时触发
             */
            void SetTimeoutCallback(int timeout, UdpSocketTimeoutCallback &&cb);

            /**
             * @brief 超时处理函数
             * 当连接超时时被调用
             */
            void OnTimeout();

            /**
             * @brief 错误处理函数
             * @param msg 错误信息
             */
            void OnError(const std::string &msg) override;

            /**
             * @brief 读事件处理函数
             * 当有数据可读时被调用
             */
            void OnRead() override;

            /**
             * @brief 写事件处理函数
             * 当可以写入数据时被调用
             */
            void OnWrite() override;

            /**
             * @brief 关闭事件处理函数
             * 当连接关闭时被调用
             */
            void OnClose() override;

            /**
             * @brief 发送数据
             * @param list 待发送的数据缓冲区列表
             */
            void Send(std::list<UdpBufferNodePtr> &list);

            /**
             * @brief 发送数据
             * @param buff 数据缓冲区指针
             * @param size 数据大小
             * @param addr 目标地址结构体指针
             * @param len 地址结构体长度
             */
            void Send(const char *buff, size_t size, struct sockaddr *addr, socklen_t len);

            /**
             * @brief 启用空闲超时检查
             * @param max_time 最大空闲时间(秒)
             */
            void EnableCheckIdleTimeout(int32_t max_time);

            /**
             * @brief 强制关闭连接
             * 立即关闭连接而不等待数据发送完成
             */
            void ForceClose() override;

          private:
            /**
             * @brief 在事件循环中发送数据
             * @param list 待发送的数据缓冲区列表
             */
            void SendInLoop(std::list<UdpBufferNodePtr> &list);

            /**
             * @brief 在事件循环中发送数据
             * @param buff 数据缓冲区指针
             * @param size 数据大小
             * @param saddr 目标地址结构体指针
             * @param len 地址结构体长度
             */
            void SendInLoop(const char *buff, size_t size, struct sockaddr *saddr, socklen_t len);
            /**
             * @brief 延长连接生命周期
             * 重置超时计时器
             */
            void ExtendLife();

            std::list<UdpBufferNodePtr> buffer_list_; ///< 待发送的UDP数据缓冲区列表
            bool closed_{false};                      ///< 连接是否已关闭标志，默认false
            int32_t max_idle_time_{30}; ///< 最大空闲超时时间(秒)，默认30秒
            std::weak_ptr<UdpTimeoutEntry> timeout_entry_; ///< 超时事件条目
            MsgBuffer message_buffer_;                     ///< 接收消息缓冲区
            int32_t message_buffer_size_{65535};  ///< 消息缓冲区大小，默认65535字节
            UdpSocketMessageCallback message_cb_; ///< 消息接收回调函数
            UdpSocketWriteCompleteCallback write_complete_cb_; ///< 写完成回调函数
            UdpSocketCloseConnectionCallback close_cb_;        ///< 连接关闭回调函数
        };

        /**
         * @brief UDP超时条目结构体
         * 用于管理UDP套接字的超时事件
         */
        struct UdpTimeoutEntry
        {
          public:
            /**
             * @brief 构造函数
             * @param c UDP套接字指针
             */
            UdpTimeoutEntry(const UdpSocketPtr &c)
                : conn(c){

                  };

            ~UdpTimeoutEntry()
            {
                auto c = conn.lock();
                if (c)
                {
                    c->OnTimeout();
                }
            };

            std::weak_ptr<UdpSocket> conn; ///< UDP套接字弱指针
        };

    } // namespace network
} // namespace tmms