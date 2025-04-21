#pragma once
#include "network/base/InetAddress.h"
#include "network/net/EventLoop.h"
#include "network/net/TcpConnection.h"
#include <functional>

namespace tmms
{
    namespace network
    {
        /**
         * @brief TCP连接状态枚举
         * 定义了TCP连接的4种状态
         */
        enum
        {
            kTcpConStatusInit = 0,        ///< 初始化状态
            kTcpConStatusConnecting = 1,  ///< 连接中状态
            kTcpConStatusConnected = 2,   ///< 已连接状态
            kTcpConStatusDisConnected = 3 ///< 断开连接状态
        };

        /// @brief 连接回调函数类型
        using ConnectionCallback = std::function<void(const TcpConnectionPtr &con, bool)>;

        /**
         * @brief TCP客户端类
         * 负责建立和管理TCP客户端连接
         */
        class TcpClient : public TcpConnection
        {
          public:
            /**
             * @brief 构造函数
             * @param loop 事件循环对象
             * @param server 服务器地址
             */
            TcpClient(EventLoop *loop, const InetAddress &server);

            /**
             * @brief 连接到服务器
             */
            void Connect();

            /**
             * @brief 设置连接回调函数
             * @param cb 连接回调函数
             */
            void SetConnectCallback(const ConnectionCallback &cb);

            /**
             * @brief 设置连接回调函数
             * @param cb 连接回调函数
             */
            void SetConnectCallback(ConnectionCallback &&cb);

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
             * @brief 发送数据
             * @param list 数据链表
             */
            void Send(std::list<BufferNodePtr> &list);

            /**
             * @brief 发送数据
             * @param buff 数据缓冲区
             * @param size 数据大小
             */
            void Send(const char *buff, size_t size);

            /**
             * @brief 析构函数
             */
            virtual ~TcpClient();

          private:
            /**
             * @brief 在事件循环中连接到服务器
             */
            void ConnectInLoop();

            /**
             * @brief 更新连接状态
             */
            void UpdateConnectionStatus();

            /**
             * @brief 检查错误
             * @return 是否有错误
             */
            bool CheckError();

            InetAddress server_addr_;          ///< 服务器地址
            int32_t status_;                   ///< 连接状态
            ConnectionCallback connection_cb_; ///< 连接回调函数
        };
    } // namespace network
} // namespace tmms
