#pragma once

#include "InetAddress.h"
#include <fcntl.h>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace tmms
{
    namespace network
    {
        using InetAddressPtr = std::shared_ptr<InetAddress>;
        /**
         * @brief Socket操作封装类
         * 提供TCP/UDP socket的创建、绑定、监听、连接等操作
         */
        class SocketOpt
        {
          public:
            /**
             * @brief 构造函数
             * @param sock 已创建的socket文件描述符
             * @param v6 是否为IPv6 socket，默认为false
             */
            SocketOpt(int sock, bool v6 = false);
            ~SocketOpt() = default;

            /**
             * @brief 创建非阻塞TCP socket
             * @param family 地址族(AF_INET或AF_INET6)
             * @return 成功返回socket文件描述符，失败返回-1
             */
            static int CreateNonBlockingTcpSocket(int family);

            /**
             * @brief 创建非阻塞UDP socket
             * @param family 地址族(AF_INET或AF_INET6)
             * @return 成功返回socket文件描述符，失败返回-1
             */
            static int CreateNonBlockingUdpSocket(int family);

            /**
             * @brief 绑定地址到socket
             * @param localaddr 要绑定的本地地址
             * @return 成功返回0，失败返回-1
             */
            int BindAddress(const InetAddress &localaddr);

            /**
             * @brief 开始监听socket连接
             * @return 成功返回0，失败返回-1
             */
            int Listen();

            /**
             * @brief 接受新连接
             * @param peeraddr 输出参数，保存对端地址
             * @return 成功返回新连接的socket文件描述符，失败返回-1
             */
            int Accept(InetAddress *peeraddr);

            /**
             * @brief 连接到指定地址
             * @param addr 要连接的对端地址
             * @return 成功返回0，失败返回-1
             */
            int Connect(const InetAddress &addr);

            /**
             * @brief 获取socket连接的本地地址
             * @return 本地地址
             */
            InetAddressPtr GetLocalAddress();

            /**
             * @brief 获取socket连接的对端地址
             * @return 对端地址
             */
            InetAddressPtr GetPeerAddress();

            
            /**
             * @brief 设置TCP_NODELAY选项，禁用Nagle算法
             * @param on true启用(禁用Nagle)，false禁用
             * @note 适用于需要低延迟的小数据包场景
             */
            void SetTcpNoDelay(bool on);
            
            /**
             * @brief 设置SO_REUSEADDR选项，允许重用本地地址
             * @param on true启用，false禁用
             * @note 适用于服务器快速重启场景
             */
            void SetReuseAddr(bool on);
            
            /**
             * @brief 设置SO_REUSEPORT选项，允许多个socket绑定相同端口
             * @param on true启用，false禁用
             * @note 适用于多进程/线程服务器负载均衡
             */
            void SetReusePort(bool on);
            
            /**
             * @brief 设置SO_KEEPALIVE选项，启用TCP保活机制
             * @param on true启用，false禁用
             * @note 适用于需要检测连接存活的场景
             */
            void SetKeepAlive(bool on);
            
            /**
             * @brief 设置O_NONBLOCK选项，设置socket为非阻塞模式
             * @param on true启用非阻塞，false禁用
             * @note 适用于异步I/O编程模型
             */
            void SetNonBlocking(bool on);

          private:
            int sock_{-1};      ///< socket文件描述符
            bool is_v6_{false}; ///< 是否为IPv6 socket
        };
    } // namespace network
} // namespace tmms