#pragma once
#include "Packet.h"
#include "base/NonCopyable.h"
#include "network/net/TcpConnection.h"
#include <memory>

namespace tmms
{
    namespace mm
    {
        using namespace network;

        /**
         * @brief 多媒体处理器基类，用于处理多媒体连接和数据传输
         *
         * MMediaHandler 类作为多媒体处理的基础抽象类，定义了处理网络连接和数据传输的标准接口。
         * 该类继承自 NonCopyable，禁止拷贝构造和赋值操作，确保每个处理器实例的唯一性。
         * 所有虚函数都是纯虚函数，需要由派生类实现具体的处理逻辑。
         */
        class MMediaHandler : public base::NonCopyable
        {
          public:
            /**
             * @brief 处理新的网络连接
             * @param conn 新建立的 TCP 连接的智能指针
             *
             * 当新的客户端连接建立时，该函数被调用。派生类应该在此函数中
             * 实现连接初始化、资源分配等操作。
             */
            virtual void OnNewConnection(const TcpConnectionPtr &conn) = 0;

            /**
             * @brief 处理连接断开或销毁事件
             * @param conn 即将销毁的 TCP 连接的智能指针
             *
             * 当客户端连接断开或需要销毁时调用。派生类应该在此函数中
             * 实现资源释放、状态清理等操作。
             */
            virtual void OnConnectionDestroy(const TcpConnectionPtr &conn) = 0;

            /**
             * @brief 处理接收到的数据（常量引用方式）
             * @param conn 接收数据的 TCP 连接智能指针
             * @param data 接收到的数据包的智能指针（常量引用）
             *
             * 用于处理从客户端接收到的数据，数据以只读方式提供。
             * 适用于不需要修改或转移数据所有权的场景。
             */
            virtual void OnRecv(const TcpConnectionPtr &conn, const PacketPtr &data) = 0;

            /**
             * @brief 处理接收到的数据（移动语义方式）
             * @param conn 接收数据的 TCP 连接智能指针
             * @param data 接收到的数据包的智能指针（右值引用）
             *
             * 用于处理从客户端接收到的数据，通过移动语义转移数据所有权。
             * 适用于需要存储或转发数据的场景，可以避免不必要的数据拷贝。
             */
            virtual void OnRecv(const TcpConnectionPtr &conn, PacketPtr &&data) = 0;

            /**
             * @brief 处理连接活跃状态事件
             * @param conn 处于活跃状态的连接智能指针
             *
             * 当连接处于活跃状态时被调用，可用于心跳检测、
             * 连接保活等场景。派生类可以在此实现相应的活跃状态处理逻辑。
             */
            virtual void OnActive(const ConnectionPtr &conn) = 0;
        };
    } // namespace mm
} // namespace tmms