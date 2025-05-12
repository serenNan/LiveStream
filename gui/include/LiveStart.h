#pragma once

#include <QObject>
#include <QThread>
#include <atomic>

// 前向声明
namespace tmms
{
    namespace base
    {
        class Logger;
    }
} // namespace tmms

/**
 * @brief 直播服务工作线程类，用于在后台执行直播服务
 */
class LiveServiceWorker : public QObject
{
    Q_OBJECT

  public:
    explicit LiveServiceWorker(QObject *parent = nullptr);
    ~LiveServiceWorker();

  public slots:
    /**
     * @brief 启动直播服务的工作方法
     */
    void doWork();

  signals:
    /**
     * @brief 服务状态变化信号
     * @param running 是否运行
     */
    void statusChanged(bool running);

    /**
     * @brief 服务日志信号
     * @param level 日志级别
     * @param message 日志消息
     */
    void logMessage(int level, const QString &message);
};

/**
 * @brief 直播服务启动类，用于管理直播服务的启动和停止
 */
class LiveStart : public QObject
{
    Q_OBJECT

  public:
    /**
     * @brief 获取LiveStart单例
     * @return LiveStart实例的引用
     */
    static LiveStart &instance();

    /**
     * @brief 启动直播服务
     */
    void startLiveService();

    /**
     * @brief 停止直播服务
     */
    void stopLiveService();

    /**
     * @brief 检查服务是否正在运行
     * @return 如果服务正在运行返回true，否则返回false
     */
    bool isRunning() const;

  signals:
    /**
     * @brief 服务日志信号
     * @param level 日志级别
     * @param message 日志消息
     */
    void logMessage(int level, const QString &message);

  private:
    LiveStart(QObject *parent = nullptr);
    ~LiveStart();
    LiveStart(const LiveStart &) = delete;
    LiveStart &operator=(const LiveStart &) = delete;

    QThread m_workerThread;        ///< 工作线程
    LiveServiceWorker *m_worker;   ///< 工作对象
    std::atomic<bool> m_isRunning; ///< 服务是否运行的标志
};