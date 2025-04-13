#include "TaskManager.h"
#include "TTime.h"
#include <cstdint>

using namespace tmms::base;

void TaskManager::OnWork()
{
    std::lock_guard<std::mutex> lock(lock_);
    int64_t now = TTime::NowMS();
    for (auto iter = tasks_.begin(); iter != tasks_.end();)
    {
        if ((*iter)->When() < now)
        {
            (*iter)->Run();
            // 再次检查任务时间(判断是否是单次任务)
            if ((*iter)->When() < now)
            {
                iter = tasks_.erase(iter);
                continue;
            }
        }
        iter++;
    }
}

bool TaskManager::Add(TaskPtr &task)
{
    std::lock_guard<std::mutex> lock(lock_);
    auto iter = tasks_.find(task);
    if (iter != tasks_.end())
    {
        return false;
    }
    tasks_.emplace(task);
    return true;
}

bool TaskManager::Del(TaskPtr &task)
{
    std::lock_guard<std::mutex> lock(lock_);
    tasks_.erase(task);
    return true;
}