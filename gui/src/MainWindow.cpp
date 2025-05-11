#include "MainWindow.h"
#include "LiveStart.h"
#include "base/ConfigManager.h"
#include "base/LogStream.h"
#include <QDateTime>
#include <QDebug>
#include <QMessageBox>

using namespace tmms::base;

/**
 * @brief 构造函数
 * @param parent 父窗口指针
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui_MainWindow), m_isRunning(false)
{
    ui->setupUi(this);
    setWindowTitle("直播流服务控制台");

    // 创建状态更新定时器
    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &MainWindow::updateStatus);
    m_statusTimer->start(1000); // 每秒更新一次状态

    // 连接信号和槽
    connectSignalsSlots();

    // 初始化配置信息显示
    initConfigInfo();

    // 初始化状态显示
    updateStatus();
}

/**
 * @brief 析构函数
 */
MainWindow::~MainWindow()
{
    // 停止服务
    if (LiveStart::instance().isRunning())
    {
        LiveStart::instance().stopLiveService();
    }

    delete ui;
}

/**
 * @brief 连接信号和槽
 */
void MainWindow::connectSignalsSlots()
{
    // 连接LiveStart的日志信号
    connect(&LiveStart::instance(), &LiveStart::logMessage, this, &MainWindow::handleServiceLog);
}

/**
 * @brief 开始/停止按钮点击事件处理
 */
void MainWindow::on_startButton_clicked()
{
    if (!m_isRunning)
    {
        // 更新UI状态
        ui->startButton->setEnabled(false);
        ui->startButton->setText("正在启动...");
        // 只修改文本颜色，保留原有背景样式
        ui->startButton->setStyleSheet(
            "QPushButton { color: red; background-color: #4CAF50; border-radius: 5px; }"
            "QPushButton:hover { background-color: #45a049; }"
            "QPushButton:pressed { background-color: #3e8e41; }");

        // 启动服务
        logMessage(kInfo, "正在启动直播服务...");
        LiveStart::instance().startLiveService();
    }
    else
    {
        // 更新UI状态
        ui->startButton->setEnabled(false);
        ui->startButton->setText("正在停止...");
        // 只修改文本颜色，保留原有背景样式
        ui->startButton->setStyleSheet(
            "QPushButton { color: white; background-color: #f44336; border-radius: 5px; }"
            "QPushButton:hover { background-color: #e53935; }"
            "QPushButton:pressed { background-color: #d32f2f; }");

        // 停止服务
        logMessage(kInfo, "正在停止直播服务...");
        LiveStart::instance().stopLiveService();
    }
}

/**
 * @brief 更新状态UI
 */
void MainWindow::updateStatus()
{
    bool serviceRunning = LiveStart::instance().isRunning();

    // 如果状态发生变化，更新UI
    if (serviceRunning != m_isRunning)
    {
        m_isRunning = serviceRunning;
        ui->startButton->setEnabled(true);
        ui->startButton->setText(m_isRunning ? "停止" : "启动");

        // 根据状态设置按钮样式
        if (m_isRunning)
        {
            // 运行中状态 - 红色停止按钮
            ui->startButton->setStyleSheet(
                "QPushButton { color: white; background-color: #f44336; border-radius: 5px; }"
                "QPushButton:hover { background-color: #e53935; }"
                "QPushButton:pressed { background-color: #d32f2f; }");
            ui->statusValueLabel->setText("运行中");
            ui->statusValueLabel->setStyleSheet("color: green;");
        }
        else
        {
            // 停止状态 - 绿色启动按钮
            ui->startButton->setStyleSheet(
                "QPushButton { color: white; background-color: #4CAF50; border-radius: 5px; }"
                "QPushButton:hover { background-color: #45a049; }"
                "QPushButton:pressed { background-color: #3e8e41; }");
            ui->statusValueLabel->setText("已停止");
            ui->statusValueLabel->setStyleSheet("color: red;");
        }
    }

    // 更新状态栏消息
    statusBar()->showMessage(m_isRunning ? "服务运行中..." : "服务已停止");
}

/**
 * @brief 显示关于对话框
 */
void MainWindow::showAboutDialog()
{
    QMessageBox::about(this, "关于直播流服务控制台",
                       "<h3>直播流服务控制台 v1.0</h3>"
                       "<p>本应用程序提供直播流服务的管理和控制功能</p>"
                       "<p>Copyright © 2023 TMMS. All rights reserved.</p>");
}

/**
 * @brief 退出按钮点击事件处理
 */
void MainWindow::on_actionExit_triggered()
{
    QApplication::quit();
}

/**
 * @brief 关于按钮点击事件处理
 */
void MainWindow::on_actionAbout_triggered()
{
    showAboutDialog();
}

/**
 * @brief 清空日志按钮点击事件处理
 */
void MainWindow::on_clearLogButton_clicked()
{
    // 显示警告确认对话框
    QMessageBox::StandardButton reply =
        QMessageBox::warning(this, "确认清空日志", "确定要清空所有日志记录吗？这个操作无法撤销。",
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        clearLog();
    }
}

/**
 * @brief 清空日志
 */
void MainWindow::clearLog()
{
    ui->logTextEdit->clear();
    logMessage(kInfo, "日志已清空");
}

/**
 * @brief 初始化配置信息显示
 */
void MainWindow::initConfigInfo()
{
    // 尝试获取配置信息
    ConfigPtr config = sConfigManager->GetConfig();
    if (config)
    {
        LogInfoPtr logInfo = config->GetLogInfo();
        if (logInfo)
        {
            ui->logPathEdit->setText(QString::fromStdString(logInfo->path + logInfo->name));
        }
    }
}

/**
 * @brief 处理服务日志消息
 * @param level 日志级别
 * @param message 日志消息
 */
void MainWindow::handleServiceLog(int level, const QString &message)
{
    // 记录日志消息
    logMessage(level, message);
}

/**
 * @brief 记录日志消息
 * @param level 日志级别
 * @param message 日志消息
 */
void MainWindow::logMessage(int level, const QString &message)
{
    // 日志级别名称
    QString levelName;
    QString color;

    switch (level)
    {
    case kTrace:
        levelName = "TRACE";
        color = "black";
        break;
    case kDebug:
        levelName = "DEBUG";
        color = "blue";
        break;
    case kInfo:
        levelName = "INFO";
        color = "green";
        break;
    case kWarn:
        levelName = "WARN";
        color = "darkorange";
        break;
    case kError:
        levelName = "ERROR";
        color = "red";
        break;
    default:
        levelName = "UNKNOWN";
        color = "black";
        break;
    }

    // 获取当前时间
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    // 格式化日志条目
    QString logEntry = QString("<font color='gray'>[%1]</font> <font color='%2'>[%3]</font> %4")
                           .arg(timestamp, color, levelName, message);

    // 添加到日志文本框
    ui->logTextEdit->append(logEntry);

    // 临时打印到控制台
    qDebug() << logEntry;
}