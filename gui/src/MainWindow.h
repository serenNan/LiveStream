#pragma once
#include "ui_MainWindow.h"
#include <QMainWindow>
#include <QTimer>

/**
 * @brief 主窗口类，提供用户界面控制直播服务
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~MainWindow();

  private:
    Ui_MainWindow *ui;     ///< UI界面指针
    QTimer *m_statusTimer; ///< 状态更新定时器
    bool m_isRunning;      ///< 运行状态标志

    /**
     * @brief 记录日志消息
     * @param level 日志级别
     * @param message 日志消息
     */
    void logMessage(int level, const QString &message);

    /**
     * @brief 初始化配置信息显示
     */
    void initConfigInfo();

    /**
     * @brief 连接信号和槽
     */
    void connectSignalsSlots();

  private slots:
    /**
     * @brief 开始/停止按钮点击事件处理
     */
    void on_startButton_clicked();

    /**
     * @brief 更新状态UI
     */
    void updateStatus();

    /**
     * @brief 显示关于对话框
     */
    void showAboutDialog();

    /**
     * @brief 清空日志
     */
    void clearLog();

    /**
     * @brief 退出按钮点击事件处理
     */
    void on_actionExit_triggered();

    /**
     * @brief 关于按钮点击事件处理
     */
    void on_actionAbout_triggered();

    /**
     * @brief 清空日志按钮点击事件处理
     */
    void on_clearLogButton_clicked();

    /**
     * @brief 处理服务日志消息
     * @param level 日志级别
     * @param message 日志消息
     */
    void handleServiceLog(int level, const QString &message);
};