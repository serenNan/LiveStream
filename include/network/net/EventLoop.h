#pragma once
#include "Event.h"
#include "PipeEvent.h"
#include "TimingWheel.h"
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <sys/epoll.h>
#include <unordered_map>
#include <vector>
namespace tmms
{
    namespace network
    {
        using EventPtr = std::shared_ptr<Event>;
        using Func = std::function<void()>;

        /**
         * @brief 事件循环类
         * 基于epoll实现的事件循环，负责管理IO事件的分发和处理
         */
        class EventLoop
        {
          public:
            /**
             * @brief 构造函数，初始化epoll实例
             * @throws std::runtime_error 如果epoll创建失败
             */
            EventLoop();

            /**
             * @brief 析构函数，关闭epoll实例
             */
            ~EventLoop();

            /**
             * @brief 开始事件循环
             * 进入事件循环，持续监听和处理IO事件
             */
            void Loop();

            /**
             * @brief 停止事件循环
             * 设置停止标志，将在下一次事件循环时退出
             */
            void Quit();

            /**
             * @brief 添加事件
             * @param event 要添加的事件
             * @throws std::runtime_error 如果epoll_ctl添加失败
             */
            void AddEvent(const EventPtr &event);

            /**
             * @brief 删除事件
             * @param event 要删除的事件
             * @throws std::runtime_error 如果epoll_ctl删除失败
             */
            void DelEvent(const EventPtr &event);

            /**
             * @brief 允许或禁止事件的读取
             * @param event 要更新的事件
             * @param enable 是否启用事件
             * @return 是否更新成功
             * @throws std::runtime_error 如果epoll_ctl修改失败
             */
            bool EnableEventReading(const EventPtr &event, bool enable);

            /**
             * @brief 允许或禁止事件的写入
             * @param event 要更新的事件
             * @param enable 是否启用事件
             * @return 是否更新成功
             * @throws std::runtime_error 如果epoll_ctl修改失败
             */
            bool EnableEventWriting(const EventPtr &event, bool enable);

            /**
             * @brief 断言当前线程是否在事件循环线程中
             * @throws std::runtime_error 如果不在事件循环线程中则报错
             */
            void AssertInLoopThread();

            /**
             * @brief 检查当前线程是否是事件循环线程
             * @return 如果是事件循环线程返回true，否则返回false
             */
            bool IsInLoopThread() const;

            /**
             * @brief 在事件循环线程中执行函数
             * @param func 要执行的函数
             */
            void RunInLoop(const Func &func);

            /**
             * @brief 在事件循环线程中执行函数（右值引用版本）
             * @param func 要执行的函数
             */
            void RunInLoop(Func &&func);

            /**
             * @brief 向时间轮插入定时任务项
             * @param delay 延迟时间(毫秒)
             * @param entryPty 定时任务项指针
             */
            void InsertEntry(uint32_t delay, EntryPtr entryPty);

            /**
             * @brief 延迟执行回调函数(左值引用版本)
             * @param delay 延迟时间(秒)
             * @param cb 回调函数
             */
            void RunAfter(double delay, const Func &cb);

            /**
             * @brief 延迟执行回调函数(右值引用版本)
             * @param delay 延迟时间(秒)
             * @param cb 回调函数
             */
            void RunAfter(double delay, Func &&cb);

            /**
             * @brief 周期性执行回调函数(左值引用版本)
             * @param interval 执行间隔(秒)
             * @param cb 回调函数
             */
            void RunEvery(double interval, const Func &cb);

            /**
             * @brief 周期性执行回调函数(右值引用版本)
             * @param interval 执行间隔(秒)
             * @param cb 回调函数
             */
            void RunEvery(double interval, Func &&cb);

          private:
            /**
             * @brief 执行队列中的函数
             */
            void RunFunctions();

            /**
             * @brief 唤醒事件循环
             */
            void WakeUp();

            bool lopping_{false}; ///< 循环运行标志，表示事件循环是否正在运行
            int epoll_fd_{-1};    ///< epoll文件描述符，用于事件监听
            std::vector<struct epoll_event>
                epoll_events_; ///< 存储epoll事件的容器，用于接收epoll_wait返回的事件
            std::unordered_map<int, EventPtr>
                events_; ///< 存储事件的映射表，键为文件描述符，值为对应的事件对象
            std::queue<Func> functions_; ///< 待执行函数队列，用于线程间通信
            std::mutex lock_;            ///< 互斥锁，保护函数队列的线程安全
            PipeEventPtr pipe_event_;    ///< 管道事件，用于唤醒事件循环
            TimingWheel wheel_;          ///< 时间轮，用于定时任务的管理
        };
    } // namespace network
} // namespace tmms