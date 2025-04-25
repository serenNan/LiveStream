#pragma once
#include "network/net/TcpConnection.h"
#include <functional>
#include <memory>
#include <string>

namespace tmms
{
    namespace network
    {
        /**
         * @brief 消息处理回调函数类型
         * @param connection TCP连接的智能指针
         * @param message 接收到的消息内容
         */
        using TestMessageCallback =
            std::function<void(const TcpConnectionPtr &connection, const std::string &message)>;

        /**
         * @brief 测试上下文类，用于管理TCP连接的消息处理
         *
         * 该类实现了一个简单的消息解析状态机，支持消息头和消息体的解析。
         * 消息格式为：[消息长度][消息内容]
         */
        class TestContext
        {
          private:
            /**
             * @brief 消息解析状态枚举
             */
            enum ParseState
            {
                kTestContextHeader = 1, ///< 正在解析消息头（消息长度）
                kTestContextBody = 2    ///< 正在解析消息体
            };

          public:
            /**
             * @brief 构造函数
             * @param con TCP连接的智能指针
             */
            TestContext(const TcpConnectionPtr &con);

            /**
             * @brief 解析接收到的消息
             * @param buff 消息缓冲区
             * @return int 解析结果，0表示成功，负值表示错误
             */
            int ParseMessage(MsgBuffer &buff);

            /**
             * @brief 设置消息处理回调函数（左值版本）
             * @param cb 回调函数对象
             */
            void SetTestMessageCallback(const TestMessageCallback &cb);

            /**
             * @brief 设置消息处理回调函数（右值版本）
             * @param cb 回调函数对象
             */
            void SetTestMessageCallback(TestMessageCallback &&cb);

            /**
             * @brief 析构函数
             */
            ~TestContext() = default;

          private:
            TcpConnectionPtr connection_;   ///< TCP连接的智能指针
            int state_{kTestContextHeader}; ///< 当前解析状态
            int32_t message_length_{0};     ///< 当前消息的长度
            std::string message_body_;      ///< 当前消息的内容
            TestMessageCallback cb_;        ///< 消息处理回调函数
        };
    } // namespace network
} // namespace tmms
