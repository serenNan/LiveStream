#pragma once
#include <cstdint>
#include <functional>
#include <memory>
namespace tmms
{
    namespace base
    {
        class Task;
        /// @brief 任务智能指针类型
        using TaskPtr = std::shared_ptr<Task>;
        /// @brief 任务回调函数类型
        using TaskCallback = std::function<void(const TaskPtr &)>;

        /**
         * @brief 定时任务类
         * @details 封装了定时执行的回调任务，支持设置执行间隔时间
         */
        class Task : public std::enable_shared_from_this<Task>
        {
          public:
            /**
             * @brief 构造函数(左值引用版本)
             * @param cb 任务回调函数
             * @param interval 任务执行间隔时间(毫秒)
             */
            Task(const TaskCallback &cb, int64_t interval);
            /**
             * @brief 构造函数(右值引用版本)
             * @param cb 任务回调函数
             * @param interval 任务执行间隔时间(毫秒)
             */
            Task(const TaskCallback &&cb, int64_t interval);

            /**
             * @brief 执行任务
             */
            void Run();
            /**
             * @brief 重新启动任务
             */
            void Restart();
            /**
             * @brief 获取任务下次执行时间
             * @return int64_t 下次执行时间戳(毫秒)
             */
            int64_t When() const
            {
                return when_;
            }

          private:
            int64_t interval_{0}; ///< 任务执行间隔时间(毫秒)
            int64_t when_{0};     ///< 下次执行时间戳(毫秒)
            TaskCallback cb_;     ///< 任务回调函数
        };
    } // namespace base
} // namespace tmms