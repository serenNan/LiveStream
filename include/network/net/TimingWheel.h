#pragma once

#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <unordered_set>
#include <vector>

namespace tmms
{
    namespace network
    {
        using EntryPtr = std::shared_ptr<void>;
        using WheelEntry = std::unordered_set<EntryPtr>;
        using Wheel = std::deque<WheelEntry>;
        using Wheels = std::vector<Wheel>;
        using Func = std::function<void()>;

        const int kTimingMinute = 60;
        const int kTimingHour = 60 * 60;
        const int kTimingDay = 60 * 60 * 24;
        enum TimingType
        {
            kTimingTypeSecond = 0,
            kTimingTypeMinute = 1,
            kTimingTypeHour = 2,
            kTimingTypeDay = 3,
        };
        class CallbackEntry
        {
          public:
            CallbackEntry(const Func &cb) : cb_(cb)
            {
            }
            ~CallbackEntry()
            {
                if (cb_)
                {
                    cb_();
                }
            }

          private:
            Func cb_;
        };

        using CallbackEntryPtr = std::shared_ptr<CallbackEntry>;

        class TimingWheel
        {
          public:
            /**
             * @brief 构造函数，初始化时间轮
             */
            TimingWheel();

            /**
             * @brief 析构函数，清理资源
             */
            ~TimingWheel();

            /**
             * @brief 插入定时任务
             * @param delay 延迟时间(秒)
             * @param entryPty 要插入的任务指针
             */
            void InsertEntry(uint32_t delay, EntryPtr entryPty);

            /**
             * @brief 定时器触发时调用
             * @param now 当前时间戳
             */
            void OnTimer(int64_t now);

            /**
             * @brief 弹出到期的任务
             * @param bq 任务队列
             */
            void PopUp(Wheel &bq);

            /**
             * @brief 延迟执行函数
             * @param delay 延迟时间(秒)
             * @param cb 回调函数
             */
            void RunAfter(double delay, const Func &cb);

            /**
             * @brief 延迟执行函数(右值引用版本)
             * @param delay 延迟时间(秒)
             * @param cb 回调函数
             */
            void RunAfter(double delay, Func &&cb);

            /**
             * @brief 周期性执行函数
             * @param interval 执行间隔(秒)
             * @param cb 回调函数
             */
            void RunEvery(double interval, const Func &cb);

            /**
             * @brief 周期性执行函数(右值引用版本)
             * @param interval 执行间隔(秒)
             * @param cb 回调函数
             */
            void RunEvery(double interval, Func &&cb);

          private:
            /**
             * @brief 插入秒级定时任务
             * @param delay 延迟时间(秒)
             * @param entryPty 要插入的任务指针
             */
            void InsertSecondEntry(uint32_t delay, EntryPtr entryPty);

            /**
             * @brief 插入分钟级定时任务
             * @param delay 延迟时间(分钟)
             * @param entryPty 要插入的任务指针
             */
            void InsertMinuteEntry(uint32_t delay, EntryPtr entryPty);

            /**
             * @brief 插入小时级定时任务
             * @param delay 延迟时间(小时)
             * @param entryPty 要插入的任务指针
             */
            void InsertHourEntry(uint32_t delay, EntryPtr entryPty);

            /**
             * @brief 插入天级定时任务
             * @param delay 延迟时间(天)
             * @param entryPty 要插入的任务指针
             */
            void InsertDayEntry(uint32_t delay, EntryPtr entryPty);
            Wheels wheels_;      ///< 时间轮容器，存储不同时间粒度的任务队列
            int64_t last_ts_{0}; ///< 上次触发时间戳
            uint64_t tick_{0};   ///< 当前tick计数
        };
    } // namespace network
} // namespace tmms