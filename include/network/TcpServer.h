#pragma once
#include "Connection.h"
#include "network/base/InetAddress.h"
#include "network/net/Acceptor.h"
#include "network/net/EventLoop.h"
#include "network/net/TcpConnection.h"
#include <functional>
#include <memory>
#include <unordered_set>

/// @file TcpServer.h
/// @brief TCP服务器实现，负责监听端口、接受新连接和管理现有连接

namespace tmms
{
    namespace network
    {
        // 新连接回调函数类型
        using NewConnectionCallback = std::function<void(const TcpConnectionPtr &)>;

        // 连接销毁回调函数类型
        using DestroyConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
        /**
         * @class TcpServer
         * @brief TCP服务器类，负责监听指定端口并管理TCP连接
         */
        class TcpServer
        {
          public:
            /**
             * @brief 构造函数，创建一个TCP服务器实例
             * @param loop 事件循环指针，用于处理网络IO事件
             * @param addr 监听地址，包含IP和端口信息
             * @note 服务器创建后需要调用Start()方法开始监听
             */
            TcpServer(EventLoop *loop, const InetAddress &addr);

            /**
             * @brief 设置新连接回调函数
             * @param cb 回调函数，当有新连接建立时触发
             */
            void SetNewConnectionCallback(const NewConnectionCallback &cb);

            /**
             * @brief 设置新连接回调函数
             * @param cb 回调函数，当有新连接建立时触发
             */
            void SetNewConnectionCallback(NewConnectionCallback &&cb);

            /**
             * @brief 设置连接销毁回调函数
             * @param cb 回调函数，当连接关闭或销毁时触发
             */
            void SetDestroyConnectionCallback(const DestroyConnectionCallback &cb);

            /**
             * @brief 设置连接销毁回调函数
             * @param cb 回调函数，当连接关闭或销毁时触发
             */
            void SetDestroyConnectionCallback(DestroyConnectionCallback &&cb);

            /**
             * @brief 设置连接活动状态回调函数
             * @param cb 回调函数，当连接状态变化时触发
             */
            void SetActiveCallback(const ActiveCallback &cb);

            /**
             * @brief 设置连接活动状态回调函数
             * @param cb 回调函数，当连接状态变化时触发
             */
            void SetActiveCallback(ActiveCallback &&cb);

            /**
             * @brief 设置写完成回调函数
             * @param cb 回调函数，当数据写入完成时触发
             */
            void SetWriteCompleteCallback(const WriteCompleteCallback &cb);

            /**
             * @brief 设置写完成回调函数
             * @param cb 回调函数，当数据写入完成时触发
             */
            void SetWriteCompleteCallback(WriteCompleteCallback &&cb);

            /**
             * @brief 设置消息接收回调函数
             * @param cb 回调函数，当收到新消息时触发
             */
            void SetMessageCallback(const MessageCallback &cb);

            /**
             * @brief 设置消息接收回调函数
             * @param cb 回调函数，当收到新消息时触发
             */
            void SetMessageCallback(MessageCallback &&cb);

            /**
             * @brief 接受新连接的处理函数
             * @param fd 新连接的socket文件描述符
             * @param addr 新连接的地址信息
             */
            void OnAccept(int fd, const InetAddress &addr);

            /**
             * @brief 连接关闭的处理函数
             * @param con 要关闭的连接指针
             */
            void OnConnectionClose(const TcpConnectionPtr &con);

            /**
             * @brief 启动服务器，开始监听端口
             */
            virtual void Start();

            /**
             * @brief 停止服务器，关闭所有连接
             */
            virtual void Stop();

            virtual ~TcpServer();

          private:
            EventLoop *loop_{nullptr};                         ///< 事件循环指针
            InetAddress addr_;                                 ///< 监听地址
            std::shared_ptr<Acceptor> acceptor_;               ///< 接收器对象
            NewConnectionCallback new_connection_cb_;          ///< 新连接回调函数
            std::unordered_set<TcpConnectionPtr> connections_; ///< 当前管理的所有连接
            MessageCallback message_cb_;                       ///< 消息接收回调函数
            ActiveCallback active_cb_;                         ///< 连接活动状态回调函数
            WriteCompleteCallback write_complete_cb_;          ///< 写完成回调函数
            DestroyConnectionCallback destroy_connection_cb_;  ///< 连接销毁回调函数
        };
    } // namespace network
} // namespace tmms