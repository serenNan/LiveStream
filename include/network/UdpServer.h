#pragma once
#include "network/base/InetAddress.h"
#include "network/net/EventLoop.h"
#include "network/net/UdpSocket.h"

namespace tmms
{
    namespace network
    {
        /**
         * @brief UDP服务器类，继承自UdpSocket
         *
         * 实现UDP服务器的基本功能，包括启动、停止和套接字管理
         */
        class UdpServer : public UdpSocket
        {
          public:
            /**
             * @brief 构造函数
             * @param loop 事件循环对象指针
             * @param server 服务器地址对象
             */
            UdpServer(EventLoop *loop, const InetAddress &server);

            /**
             * @brief 启动服务器
             */
            void Start();

            /**
             * @brief 停止服务器
             */
            void Stop();

            /**
             * @brief 析构函数
             */
            virtual ~UdpServer();

          private:
            /**
             * @brief 打开服务器套接字
             *
             * 初始化套接字并绑定到指定地址
             */
            void Open();

            InetAddress server_; ///< 服务器绑定地址
        };
    } // namespace network
} // namespace tmms