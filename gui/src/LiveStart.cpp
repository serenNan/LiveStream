#include "LiveStart.h"
#include "base/Config.h"
#include "base/ConfigManager.h"
#include "base/FileLogManager.h"
#include "base/LogStream.h"
#include "base/TaskManager.h"
#include "live/LiveService.h"
#include <QCoreApplication>
#include <QDebug>
#include <iostream>
#include <thread>

using namespace tmms::base;
using namespace tmms::mm;
using namespace tmms::live;

// ===================== LiveServiceWorker 实现 =====================

LiveServiceWorker::LiveServiceWorker(QObject *parent) : QObject(parent)
{
}

LiveServiceWorker::~LiveServiceWorker()
{
}

void LiveServiceWorker::doWork()
{
    // 发送状态变化信号
    emit statusChanged(true);
    emit logMessage(kInfo, "正在初始化直播服务...");

    // 创建全局 Logger 对象，初始为空
    g_logger = new Logger(nullptr);

    // 设置日志级别为 Trace
    g_logger->SetLogLevel(kTrace);

    // 加载配置文件
    if (!sConfigManager->LoadConfig("../bin/config/config.json"))
    {
        std::cerr << " Load config file failed." << std::endl;
        emit logMessage(kError, "加载配置文件失败，服务无法启动");
        emit statusChanged(false);
        return;
    }

    // 获取配置对象
    ConfigPtr config = sConfigManager->GetConfig();

    // 获取日志信息
    LogInfoPtr log_info = config->GetLogInfo();

    emit logMessage(kInfo, QString("日志级别: %1, 路径: %2, 名称: %3")
                               .arg(log_info->level)
                               .arg(QString::fromStdString(log_info->path))
                               .arg(QString::fromStdString(log_info->name)));

    // 获取文件日志
    FileLogPtr log = sFileLogManager->GetFileLog(log_info->path + log_info->name);

    if (!log)
    {
        std::cerr << "Cannot open log, exit." << std::endl;
        emit logMessage(kError, "无法打开日志文件，服务无法启动");
        emit statusChanged(false);
        return;
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
    emit logMessage(kInfo, "直播服务已启动");

    // 主循环
    bool running = true;
    while (running)
    {
        // 执行任务管理器中的工作
        sTaskManager->OnWork();

        // 使用事件处理，让Qt的信号槽可以正常工作
        QCoreApplication::processEvents();

        // 线程休眠 50 毫秒
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // 检查线程是否应该停止
        QThread *currentThread = QThread::currentThread();
        if (currentThread && currentThread->isInterruptionRequested())
        {
            running = false;
            emit logMessage(kInfo, "收到停止请求，正在停止服务...");
        }
    }

    // 服务停止
    emit logMessage(kInfo, "直播服务已停止");
    emit statusChanged(false);
}

// ===================== LiveStart 实现 =====================

LiveStart &LiveStart::instance()
{
    static LiveStart instance;
    return instance;
}

LiveStart::LiveStart(QObject *parent) : QObject(parent), m_isRunning(false), m_worker(nullptr)
{
}

LiveStart::~LiveStart()
{
    if (m_isRunning.load())
    {
        stopLiveService();
    }
}

void LiveStart::startLiveService()
{
    if (m_isRunning.load())
    {
        return;
    }

    // 创建工作对象
    m_worker = new LiveServiceWorker();
    m_worker->moveToThread(&m_workerThread);

    // 连接信号和槽
    connect(&m_workerThread, &QThread::started, m_worker, &LiveServiceWorker::doWork);
    connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &LiveServiceWorker::statusChanged, this,
            [this](bool running) { m_isRunning.store(running); });

    // 转发日志信号
    connect(m_worker, &LiveServiceWorker::logMessage, this, &LiveStart::logMessage);

    // 启动线程
    m_workerThread.start();
}

void LiveStart::stopLiveService()
{
    if (!m_isRunning.load())
    {
        return;
    }

    // 请求中断线程
    m_workerThread.requestInterruption();

    // 等待线程结束
    if (m_workerThread.isRunning())
    {
        if (!m_workerThread.wait(3000))
        {                               // 等待最多3秒
            m_workerThread.terminate(); // 如果等待超时，强制终止线程
            m_workerThread.wait();      // 等待线程真正结束
        }
    }

    m_isRunning.store(false);
    m_worker = nullptr; // 工作对象会自动删除
}

bool LiveStart::isRunning() const
{
    return m_isRunning.load();
}
