#pragma once
#include "EventLoop.h"
#include "base/NonCopyable.h"
#include <condition_variable>
#include <future>
#include <mutex>
#include <thread>

namespace tmms
{
    namespace network
    {
        /**
         * @brief 事件循环线程类
         * 封装了一个线程和一个事件循环，用于在独立线程中运行事件循环
         */
        class EventLoopThread : public base::NonCopyable
        {
          public:
            /**
             * @brief 构造函数
             * 初始化事件循环线程相关成员变量
             */
            EventLoopThread();

            /**
             * @brief 析构函数
             * 确保线程安全退出并清理资源
             */
            ~EventLoopThread();

            /**
             * @brief 启动事件循环线程
             * 创建新线程并启动事件循环
             * @note 线程安全，可多次调用但仅第一次有效
             */
            void Run();

            /**
             * @brief 获取事件循环指针
             * @return EventLoop* 返回事件循环指针
             * @note 线程安全，可在任意线程调用
             */
            EventLoop *Loop() const;

            /**
             * @brief 获取线程对象引用
             * @return std::thread& 返回线程对象引用
             * @note 主要用于线程管理操作
             */
            std::thread &Thread();

          private:
            /**
             * @brief 启动事件循环
             */
            void StartEventLoop();

            EventLoop *loop_{nullptr};          ///< 事件循环指针
            std::thread thread_;                ///< 事件循环线程
            bool running_{false};               ///< 线程运行状态标志
            std::mutex lock_;                   ///< 互斥锁
            std::condition_variable condition_; ///< 条件变量
            std::once_flag once_;               ///< 一次性标志
            std::promise<int> promise_loop_;    ///< 用于线程间通信的promise
        };
    } // namespace network

} // namespace tmms