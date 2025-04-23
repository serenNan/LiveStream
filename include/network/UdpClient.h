#pragma once
#include "Connection.h"
#include "network/net/UdpSocket.h"
#include <functional>

namespace tmms
{
    namespace network
    {
        /// @brief 连接回调函数类型
        /// 当连接状态改变时调用的回调函数
        using ConnectedCallback = std::function<void(const UdpSocketPtr &, bool)>;

        /**
         * @brief UDP客户端类
         * 继承自UdpSocket，实现UDP客户端功能
         */
        class UdpClient : public UdpSocket
        {
          public:
            /**
             * @brief 构造函数
             * @param loop 事件循环对象
             * @param server 服务器地址
             */
            UdpClient(EventLoop *loop, const InetAddress &server);

            /**
             * @brief 连接到服务器
             * 非线程安全，只能在事件循环线程中调用
             */
            void Connect();

            /**
             * @brief 设置连接回调函数
             * @param cb 连接状态改变时的回调函数
             */
            void SetConnectedCallback(const ConnectedCallback &cb);

            /**
             * @brief 设置连接回调函数(移动语义)
             * @param cb 连接状态改变时的回调函数
             */
            void SetConnectedCallback(ConnectedCallback &&cb);

            /**
             * @brief 在事件循环中连接到服务器
             * 线程安全，可在任意线程调用
             */
            void ConnectInLoop();

            /**
             * @brief 发送数据
             * @param list 待发送的数据缓冲区列表
             */
            void Send(std::list<BufferNodePtr> &list);

            /**
             * @brief 发送数据
             * @param buff 数据缓冲区指针
             * @param size 数据大小
             */
            void Send(const char *buff, size_t size);

            /**
             * @brief 关闭事件处理函数
             * 当连接关闭时被调用
             */
            void OnClose() override;

            /**
             * @brief 析构函数
             */
            virtual ~UdpClient();

          private:
            bool connected_{false};                           ///< 连接状态标志
            InetAddress server_addr_;                         ///< 服务器地址
            ConnectedCallback connected_cb_;                  ///< 连接状态改变回调函数
            struct sockaddr_in6 sock_addr_;                   ///< 服务器地址结构体
            socklen_t sock_len_{sizeof(struct sockaddr_in6)}; ///< 地址结构体长度
        };
    } // namespace network

} // namespace tmms