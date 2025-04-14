#include "ConfigManager.h"
#include "FileLog.h"
#include "FileLogManager.h"
#include "LogStream.h"
#include "Logger.h"
#include "Task.h"
#include "TaskManager.h"
#include "base/Config.h"
#include <iostream>
#include <thread>

using namespace tmms::base;

int main(int argc, const char **argv)
{
    if (!sConfigManager->LoadConfig("../bin/config/config.json"))
    {
        std::cerr << "Failed to load config" << std::endl;
    }
    ConfigPtr config = sConfigManager->GetConfig();
    LogInfoPtr log_info = config->GetLogInfo();
    std::cout << "log level:" << log_info->level << " path:" << log_info->path
              << " name:" << log_info->name << std::endl;
    FileLogPtr log = sFileLogManager->GetFileLog(log_info->path + log_info->name);
    if (!log)
    {
        std ::cerr << "Failed to get file log" << std::endl;
        return -1;
    }
    log->SetRotateType(log_info->rotate_type);
    g_logger = new Logger(log);
    g_logger->SetLogLevel(log_info->level);
    TaskPtr task = std::make_shared<Task>(
        [](const TaskPtr &task) {
            sFileLogManager->OnCheck();
            task->Restart();
        },
        1000);
    while (1)
    {
        sTaskManager->OnWork();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}