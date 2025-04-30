#include "FileLog.h"
#include "FileLogManager.h"
#include "LogStream.h"
#include "Logger.h"
#include "TTime.h"
#include "Task.h"
#include "TaskManager.h"
#include "gtest/gtest.h"
#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

using namespace tmms::base;

// 全局线程变量改为线程指针，便于管理生命周期
std::unique_ptr<std::thread> t_ptr;
// 添加原子标志控制线程运行
std::atomic<bool> g_running{false};

class LoggerTest : public ::testing::Test
{
  protected:
    void TearDown() override
    {
        // 测试结束时确保线程停止并清理资源
        g_running = false;
        if (t_ptr && t_ptr->joinable())
        {
            t_ptr->join();
        }
        t_ptr.reset();

        // 移除任务
        if (task)
        {
            sTaskManager->Del(task);
        }

        // 清理全局记录器
        delete g_logger;
        g_logger = nullptr;
    }

    TaskPtr task;
};

TEST_F(LoggerTest, BasicLogging)
{
    FileLogPtr log = sFileLogManager->GetFileLog("test.log");
    log->SetRotateType(kRotateMinute);
    g_logger = new Logger(log);
    g_logger->SetLogLevel(kTrace);

    // 存储任务指针到测试类成员变量
    task = std::make_shared<Task>(
        [](const TaskPtr &task) {
            sFileLogManager->OnCheck();
            task->Restart();
        },
        1000);
    sTaskManager->Add(task);

    // 设置运行标志并创建线程
    g_running = true;
    t_ptr = std::make_unique<std::thread>([]() {
        while (g_running)
        {
            LOG_INFO << "test info message now:" << TTime::NowMS();
            LOG_ERROR << "test error message" << TTime::NowMS();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });

    // 运行测试循环
    for (int i = 0; i < 6; i++)
    {
        sTaskManager->OnWork();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // TearDown会自动处理资源清理
}

// void TestLog()
// {
//     t = std::thread([]() {
//         while (true)
//         {
//             LOG_INFO << "test info message now:" << TTime::NowMS();
//             LOG_ERROR << "test error message" << TTime::NowMS();
//             std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 暂停500毫秒
//         }
//     });
// }

// int main(int argc, char **argv)
// {
//     FileLogPtr log = sFileLogManager->GetFileLog("test.log");
//     log->SetRotateType(kRotateMinute);
//     g_logger = new Logger(log);
//     g_logger->SetLogLevel(kTrace);
//     TaskPtr task = std::make_shared<Task>(
//         [](const TaskPtr &task) {
//             sFileLogManager->OnCheck();
//             task->Restart();
//         },
//         1000);
//     sTaskManager->Add(task);
//     TestLog();
//     for(int i = 0;i < 60;i++)
//     {
//         sTaskManager->OnWork();
//         std::this_thread::sleep_for(std::chrono::seconds(1));
//     }
//     return 0;
// }