#pragma once
#include "Event.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "NonCopyable.h"
#include <atomic>
#include <memory>
#include <vector>
namespace tmms
{
    namespace network
    {
        using EventLoopThreadPtr = std::shared_ptr<EventLoopThread>;
        class EventLoopThreadPool : public base::NonCopyable
        {
          public:
            /**
             * @brief 构造函数
             * @param thread_num 线程池中线程数量
             * @param start 线程起始索引，默认为0
             * @param cpus CPU核心数，默认为4
             * @note 会根据CPU核心数自动调整线程数量
             */
            EventLoopThreadPool(int thread_num, int start = 0, int cpus = 4);

            /**
             * @brief 析构函数
             * 安全停止所有线程并清理资源
             */
            ~EventLoopThreadPool();

            /**
             * @brief 获取所有事件循环指针
             * @return std::vector<EventLoop*> 返回所有事件循环指针的集合
             * @note 线程安全，可在任意线程调用
             */
            std::vector<EventLoop *> GetLoops() const;

            /**
             * @brief 获取下一个事件循环(轮询方式)
             * @return EventLoop* 返回下一个可用的事件循环
             * @note 线程安全，采用原子操作保证一致性
             */
            EventLoop *GetNextLoop();

            /**
             * @brief 获取线程池大小
             * @return size_t 返回线程池中线程数量
             */
            size_t Size();

            /**
             * @brief 启动线程池
             * 创建并启动所有工作线程
             * @note 线程安全，可多次调用但仅第一次有效
             */
            void Start();

          private:
            std::vector<EventLoopThreadPtr> threads_; ///< 事件循环线程集合
            std::atomic_int32_t loop_index_{0}; ///< 当前轮询索引，原子操作保证线程安全
        };
    } // namespace network
} // namespace tmms