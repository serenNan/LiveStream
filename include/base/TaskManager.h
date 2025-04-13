#pragma once

#include "NonCopyable.h"
#include "Singleton.h"
#include "Task.h"
#include <mutex>
#include <unordered_set>

namespace tmms
{
    namespace base
    {
        /**
         * @brief 任务管理器类
         * @details 管理所有定时任务的添加、删除和执行
         */
        class TaskManager : public NonCopyable
        {
          public:
            TaskManager() = default;
            ~TaskManager() = default;

            /**
             * @brief 执行所有到期任务
             */
            void OnWork();

            /**
             * @brief 添加任务
             * @param task 要添加的任务指针
             * @return bool 是否添加成功
             */
            bool Add(TaskPtr &task);

            /**
             * @brief 删除任务
             * @param task 要删除的任务指针
             * @return bool 是否删除成功
             */
            bool Del(TaskPtr &task);

          private:
            std::unordered_set<TaskPtr> tasks_; ///< 存储所有任务的集合
            std::mutex lock_;                  ///< 保护任务集合的互斥锁
        };
    } // namespace base
  } // namespace tmms
  #define sTaskManager tmms::base::Singleton<TaskManager>::Instance()