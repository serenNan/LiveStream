#pragma once

#include "Event.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketOpt.h"
#include <functional>

namespace tmms
{
    namespace network
    {
        /**
         * @brief 接受新连接的回调函数类型
         * @param sock 新连接的socket文件描述符
         * @param addr 新连接的地址信息
         */
        using AcceptCallback = std::function<void(int sock, const InetAddress &addr)>;
        /**
         * @brief 用于接受新连接的类，继承自Event
         */
        class Acceptor : public Event
        {
          public:
            /**
             * @brief 构造函数
             * @param loop 事件循环
             * @param addr 监听地址
             */
            Acceptor(EventLoop *loop, const InetAddress &addr);
            ~Acceptor();

            /**
             * @brief 设置接受新连接的回调函数
             * @param cb 回调函数
             */
            void setAcceptCallback(const AcceptCallback &cb);
            
            /**
             * @brief 设置接受新连接的回调函数（移动语义）
             * @param cb 回调函数
             */
            void setAcceptCallback(AcceptCallback &&cb);

            /**
             * @brief 开始监听
             */
            void Start();

            /**
             * @brief 停止监听
             */
            void Stop();

            /**
             * @brief 处理读事件
             */
            void OnRead() override;

            /**
             * @brief 处理错误事件
             * @param msg 错误信息
             */
            void OnError(const std::string &msg) override;

            /**
             * @brief 处理关闭事件
             */
            void OnClose() override;

          private:
            /**
             * @brief 打开监听socket
             */
            void Open();

            InetAddress addr_;               ///< 监听地址
            AcceptCallback accept_cb_;       ///< 接受新连接的回调函数
            SocketOpt *socket_opt_{nullptr}; ///< socket操作对象
        };
    } // namespace network
} // namespace tmms