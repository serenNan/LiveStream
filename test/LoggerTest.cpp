#include "base/Logger.h"
#include "base/FileLog.h"
#include "base/FileLogManager.h"
#include "base/LogStream.h"
#include "base/TTime.h"
#include "base/Task.h"
#include "base/TaskManager.h"
#include "gtest/gtest.h"
#include <chrono>
#include <thread>
using namespace tmms::base;
std::thread t;
// TEST(LoggerTest, BasicLogging)
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
//     t = std::thread([]() {
//         while (true)
//         {
//             LOG_INFO << "test info message now:" << TTime::NowMS();
//             LOG_ERROR << "test error message" << TTime::NowMS();
//             std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 暂停500毫秒
//         }
//     });
//     while (1)
//     {
//         sTaskManager->OnWork();
//         std::this_thread::sleep_for(std::chrono::milliseconds(50));
//     }
// }

void TestLog()
{
    t = std::thread([]() {
        while (true)
        {
            LOG_INFO << "test info message now:" << TTime::NowMS();
            LOG_ERROR << "test error message" << TTime::NowMS();
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 暂停500毫秒
        }
    });
}

int main(int argc, char **argv)
{
    FileLogPtr log = sFileLogManager->GetFileLog("test.log");
    log->SetRotateType(kRotateMinute);
    g_logger = new Logger(log);
    g_logger->SetLogLevel(kTrace);
    TaskPtr task = std::make_shared<Task>(
        [](const TaskPtr &task) {
            sFileLogManager->OnCheck();
            task->Restart();
        },
        1000);
    sTaskManager->Add(task);
    TestLog();
    while (1)
    {
        sTaskManager->OnWork();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}