#pragma once

#include <arpa/inet.h>
#include <bits/socket.h>
#include <cstdint>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

namespace tmms
{
    namespace network
    {
        /**
         * @brief 网络地址类，封装IPv4/IPv6地址操作
         *
         * 提供IP地址的解析、转换、判断等功能，支持IPv4和IPv6协议
         */
        class InetAddress
        {
          public:
            /**
             * @brief 构造函数，通过IP和端口创建地址
             * @param ip IP地址字符串
             * @param port 端口号
             * @param bv6 是否为IPv6地址，默认为false
             */
            InetAddress(const std::string &ip, uint16_t port, bool bv6 = false);

            /**
             * @brief 构造函数，通过主机名创建地址
             * @param host 主机名或IP地址字符串
             * @param is_v6 是否为IPv6地址，默认为false
             */
            InetAddress(const std::string &host, bool is_v6 = false);

            /**
             * @brief 默认构造函数
             */
            InetAddress() = default;

            /**
             * @brief 析构函数
             */
            ~InetAddress() = default;
            /**
             * @brief 设置主机名
             * @param host 主机名或IP地址字符串
             */
            void SetHost(const std::string &host);

            /**
             * @brief 设置IP地址
             * @param addr IP地址字符串
             */
            void SetAddr(const std::string &addr);

            /**
             * @brief 设置端口号
             * @param port 端口号
             */
            void SetPort(uint16_t port);

            /**
             * @brief 设置是否为IPv6地址
             * @param is_v6 true表示IPv6，false表示IPv4
             */
            void SetIsIPV6(bool is_v6);

            /**
             * @brief 获取IP地址字符串
             * @return IP地址字符串引用
             */
            const std::string &IP() const;

            /**
             * @brief 获取IPv4地址数值
             * @return IPv4地址的32位无符号整数
             */
            uint32_t IPV4() const;

            /**
             * @brief 获取IP地址和端口号的组合字符串
             * @return 格式为"IP:PORT"的字符串
             */
            std::string ToIpPort() const;

            
            /**
             * @brief 获取端口号的数值形式
             * @return 端口号的32位无符号整数表示
             */
            uint32_t Port() const;

            /**
             * @brief 获取socket地址结构
             * @param saddr 输出的socket地址结构指针
             */
            void GetSockAddr(struct sockaddr *saddr) const;

            /**
             * @brief 判断是否为IPv6地址
             * @return true表示IPv6，false表示IPv4
             */
            bool IsIPV6() const;

            /**
             * @brief 判断是否为公网IP
             * @return true表示公网IP，false表示内网IP
             */
            bool IsWanIp() const;

            /**
             * @brief 判断是否为局域网IP
             * @return true表示局域网IP，false表示非局域网IP
             */
            bool IsLanIp() const;

            /**
             * @brief 判断是否为回环地址
             * @return true表示回环地址，false表示非回环地址
             */
            bool IsLoopbackIp() const;
            
            /**
             * @brief 从主机名中提取IP地址和端口号
             * @param host 主机名或IP地址字符串
             * @param ip 输出的IP地址字符串
             * @param port 输出的端口号字符串
             */
            static void GetIpAndPort(const std::string &host, std::string &ip, std::string &port);

          private:
            /**
             * @brief 将IP字符串转换为IPv4数值
             * @param ip IP地址字符串
             * @return IPv4地址的32位无符号整数
             */
            uint32_t IPV4(const char *ip) const;

            std::string addr_;     ///< IP地址字符串
            std::string port_;     ///< 端口号字符串
            bool is_ipv6_ = false; ///< 是否为IPv6地址
        };
    } // namespace network
} // namespace tmms