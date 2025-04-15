#pragma once
#include <memory>
#include <string>
#include <sys/epoll.h>
namespace tmms
{
    namespace network
    {
        class EventLoop;
        /// @brief 读事件标志(EPOLLIN | EPOLLPRI | EPOLLET)
        const int kEventRead = (EPOLLIN | EPOLLPRI | EPOLLET);
        /// @brief 写事件标志(EPOLLOUT | EPOLLET)
        const int kEventWrite = (EPOLLOUT | EPOLLET);

        /**
         * @brief 事件基类
         * 封装了文件描述符和事件回调接口，用于处理IO事件
         */
        class Event : public std::enable_shared_from_this<Event>
        {
            friend class EventLoop;

          public:
            /// @brief 默认构造函数
            Event();
            /// @brief 析构函数
            ~Event();
            /**
             * @brief 构造函数
             * @param loop 所属的事件循环
             * @param fd 关联的文件描述符
             */
            Event(EventLoop *loop, int fd);

            /**
             * @brief 构造函数
             * @param loop 所属的事件循环
             */
            Event(EventLoop *loop);

            /**
             * @brief 读事件回调(纯虚函数)
             * 当文件描述符可读时被调用
             */
            virtual void OnRead() {};
            /**
             * @brief 写事件回调
             * 当文件描述符可写时被调用
             */
            virtual void OnWrite(){};
            /**
             * @brief 关闭事件回调
             * 当连接关闭时被调用
             */
            virtual void OnClose(){};
            /**
             * @brief 错误事件回调
             * @param message 错误信息
             * 当发生错误时被调用
             */
            virtual void OnError(const std::string &message){};
            /**
             * @brief 启用/禁用读事件
             * @param enable 是否启用
             * @return 是否操作成功
             */
            bool EnableReading(bool enable);
            /**
             * @brief 启用/禁用写事件
             * @param enable 是否启用
             * @return 是否操作成功
             */
            bool EnableWriting(bool enable);
            /**
             * @brief 获取关联的文件描述符
             * @return 文件描述符
             */
            int Fd() const;

          protected:
            EventLoop *loop_{nullptr}; ///< 所属的事件循环
            int fd_{-1};               ///< 关联的文件描述符
            int events_{0};           ///< 当前关注的事件标志
        };
    } // namespace network
} // namespace tmms