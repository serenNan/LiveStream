#include "base/Config.h"
#include "base/ConfigManager.h"
#include "base/FileLogManager.h"
#include "base/LogStream.h"
#include "base/TaskManager.h"
#include "live/LiveService.h"
#include <iostream>
#include <thread>

using namespace tmms::base;
using namespace tmms::mm;
using namespace tmms::live;

int main(int argc, const char **argv)
{
    // printf("hello world !\n");

    // 创建全局 Logger 对象，初始为空
    g_logger = new Logger(nullptr);

    // 设置日志级别为 Trace
    g_logger->SetLogLevel(kTrace);

    // 加载配置文件
    if (!sConfigManager->LoadConfig("../bin/config/config.json"))
    {
        // 如果加载配置文件失败，输出错误信息并返回 -1
        std::cerr << " Load config file failed." << std::endl;
        return -1;
    }

    // 获取配置对象
    ConfigPtr config = sConfigManager->GetConfig();

    // 获取日志信息
    LogInfoPtr log_info = config->GetLogInfo();

    // 输出日志级别、路径和名称
    std::cout << "Log level: " << log_info->level << ", path: " << log_info->path
              << ", name: " << log_info->name << std::endl;

    // 获取文件日志
    FileLogPtr log = sFileLogManager->GetFileLog(log_info->path + log_info->name);

    if (!log)
    {
        // 如果无法打开日志，输出错误信息并返回 -1
        std::cerr << "Cannot open log, exit." << std::endl;
        return -1;
    }

    // 设置日志的轮转类型
    log->SetRotateType(log_info->rotate_type);

    // 创建新的 Logger 对象，使用文件日志
    g_logger = new Logger(log);

    // 设置新的 Logger 的日志级别
    g_logger->SetLogLevel(log_info->level);

    // 创建一个定时任务，每 1000 毫秒执行一次
    TaskPtr task = std::make_shared<Task>(
        [](const TaskPtr &task) {
            // 执行文件检查任务
            sFileLogManager->OnCheck();
            // 重启任务
            task->Restart();
        },
        1000);

    // 将任务添加到任务管理器
    sTaskManager->Add(task);

    // 启动直播服务
    sLiveService->Start();

    while (1)
    {
        // 执行任务管理器中的工作
        sTaskManager->OnWork();
        // 线程休眠 50 毫秒
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return 0;
}