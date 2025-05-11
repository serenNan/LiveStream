#include "PushStreamDialog.h"
#include "ui_PushStreamDialog.h"
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QStyle>
#include <QTime>

/**
 * @brief 构造函数
 * @param parent 父窗口指针
 */
PushStreamDialog::PushStreamDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::PushStreamDialog), m_ffmpegProcess(nullptr), m_isStreaming(false),
      m_streamSeconds(0)
{
    ui->setupUi(this);

    // 设置窗口标题和大小
    setWindowTitle(tr("视频推流"));
    resize(700, 550);

    // 加载样式表
    QFile styleFile(":/pushstream_style.qss");
    if (styleFile.open(QFile::ReadOnly))
    {
        QString styleSheet = QLatin1String(styleFile.readAll());
        setStyleSheet(styleSheet);
        styleFile.close();
    }
    else
    {
        qDebug() << "无法加载样式表文件";
    }

    // 初始化推流按钮样式
    ui->pushButton->setProperty("status", "stopped");

    // 创建FFmpeg进程
    m_ffmpegProcess = new QProcess(this);

    // 创建计时器
    m_timeTimer = new QTimer(this);
    m_timeTimer->setInterval(1000); // 1秒更新一次

    // 连接信号和槽
    connect(m_ffmpegProcess, &QProcess::readyReadStandardOutput, this,
            &PushStreamDialog::handleProcessOutput);
    connect(m_ffmpegProcess, &QProcess::readyReadStandardError, this,
            &PushStreamDialog::handleProcessError);
    connect(m_ffmpegProcess,
            static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,
            &PushStreamDialog::handleProcessFinished);
    connect(m_timeTimer, &QTimer::timeout, this, &PushStreamDialog::updateStreamTime);
}

/**
 * @brief 析构函数
 */
PushStreamDialog::~PushStreamDialog()
{
    // 如果进程仍在运行，终止它
    if (m_ffmpegProcess && m_ffmpegProcess->state() != QProcess::NotRunning)
    {
        m_ffmpegProcess->terminate();
        m_ffmpegProcess->waitForFinished(3000); // 等待最多3秒
        if (m_ffmpegProcess->state() != QProcess::NotRunning)
        {
            m_ffmpegProcess->kill(); // 强制终止
        }
    }

    delete ui;
}

/**
 * @brief 选择视频文件按钮点击事件
 */
void PushStreamDialog::on_selectFileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("选择视频文件"), QDir::homePath(),
        tr("视频文件 (*.mp4 *.avi *.mkv *.flv *.mov);;所有文件 (*.*)"));

    if (!fileName.isEmpty())
    {
        m_selectedFile = fileName;
        ui->filePathEdit->setText(fileName);
    }
}

/**
 * @brief 开始/停止推流按钮点击事件
 */
void PushStreamDialog::on_pushButton_clicked()
{
    if (!m_isStreaming)
    {
        // 开始推流
        if (m_selectedFile.isEmpty())
        {
            QMessageBox::warning(this, tr("警告"), tr("请先选择一个视频文件"));
            return;
        }

        // 构建FFmpeg命令参数
        QStringList args = buildFFmpegArgs();

        // 开始推流
        ui->outputText->clear();
        ui->outputText->append(tr("正在启动FFmpeg推流..."));
        ui->outputText->append(tr("命令: ffmpeg ") + args.join(" "));

        m_ffmpegProcess->start("ffmpeg", args);

        if (!m_ffmpegProcess->waitForStarted(3000))
        {
            ui->outputText->append(tr("错误: 无法启动FFmpeg进程"));
            ui->statusLabel->setText(tr("状态：启动失败"));
            return;
        }

        m_isStreaming = true;
        m_streamSeconds = 0;
        m_timeTimer->start();
        ui->statusLabel->setText(tr("状态：推流中"));
        updateUIState();
    }
    else
    {
        // 停止推流
        if (m_ffmpegProcess->state() != QProcess::NotRunning)
        {
            ui->outputText->append(tr("正在停止FFmpeg推流..."));

            // 发送q命令使FFmpeg优雅退出
            m_ffmpegProcess->write("q\n");

            // 如果3秒内不退出，则强制终止
            if (!m_ffmpegProcess->waitForFinished(3000))
            {
                m_ffmpegProcess->terminate();
                if (!m_ffmpegProcess->waitForFinished(2000))
                {
                    m_ffmpegProcess->kill();
                }
                ui->outputText->append(tr("FFmpeg进程已强制终止"));
            }
        }

        m_isStreaming = false;
        m_timeTimer->stop();
        ui->statusLabel->setText(tr("状态：已停止"));
        updateUIState();
    }
}

/**
 * @brief 处理FFmpeg进程的标准输出
 */
void PushStreamDialog::handleProcessOutput()
{
    QByteArray output = m_ffmpegProcess->readAllStandardOutput();
    ui->outputText->append(QString::fromUtf8(output));
}

/**
 * @brief 处理FFmpeg进程的错误输出
 */
void PushStreamDialog::handleProcessError()
{
    QByteArray error = m_ffmpegProcess->readAllStandardError();
    ui->outputText->append(QString::fromUtf8(error));
}

/**
 * @brief 处理FFmpeg进程结束
 * @param exitCode 进程退出码
 * @param exitStatus 进程退出状态
 */
void PushStreamDialog::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit)
    {
        ui->outputText->append(tr("FFmpeg进程已正常退出，退出码: ") + QString::number(exitCode));
    }
    else
    {
        ui->outputText->append(tr("FFmpeg进程已崩溃，退出码: ") + QString::number(exitCode));
    }

    m_isStreaming = false;
    m_timeTimer->stop();
    ui->statusLabel->setText(tr("状态：已停止"));
    updateUIState();
}

/**
 * @brief 更新推流时间
 */
void PushStreamDialog::updateStreamTime()
{
    m_streamSeconds++;
    QTime time(0, 0, 0);
    time = time.addSecs(m_streamSeconds);
    ui->timeLabel->setText(tr("推流时间：") + time.toString("hh:mm:ss"));
}

/**
 * @brief 更新UI状态
 */
void PushStreamDialog::updateUIState()
{
    if (m_isStreaming)
    {
        ui->pushButton->setText(tr("停止推流"));
        ui->pushButton->setProperty("status", "running");
        ui->filePathEdit->setEnabled(false);
        ui->selectFileButton->setEnabled(false);
        ui->urlEdit->setEnabled(false);
        ui->codecCombo->setEnabled(false);
        ui->optionsEdit->setEnabled(false);
        ui->loopCheck->setEnabled(false);
    }
    else
    {
        ui->pushButton->setText(tr("开始推流"));
        ui->pushButton->setProperty("status", "stopped");
        ui->filePathEdit->setEnabled(true);
        ui->selectFileButton->setEnabled(true);
        ui->urlEdit->setEnabled(true);
        ui->codecCombo->setEnabled(true);
        ui->optionsEdit->setEnabled(true);
        ui->loopCheck->setEnabled(true);
    }

    // 强制刷新样式
    ui->pushButton->style()->unpolish(ui->pushButton);
    ui->pushButton->style()->polish(ui->pushButton);
}

/**
 * @brief 构建FFmpeg命令行参数
 * @return 参数列表
 */
QStringList PushStreamDialog::buildFFmpegArgs()
{
    QStringList args;

    // 添加循环参数
    if (ui->loopCheck->isChecked())
    {
        args << "-stream_loop" << "-1";
    }

    // 输入文件
    args << "-i" << m_selectedFile;

    // 编码选项
    int codecIndex = ui->codecCombo->currentIndex();
    switch (codecIndex)
    {
    case 0: // 拷贝编码
        args << "-c" << "copy";
        break;
    case 1: // H.264视频
        args << "-c:v" << "libx264";
        args << "-c:a" << "copy";
        break;
    case 2: // AAC音频
        args << "-c:v" << "copy";
        args << "-c:a" << "aac";
        break;
    case 3: // 自定义
        // 不添加编码选项，使用其他参数栏中指定的选项
        break;
    }

    // 其他参数
    QString options = ui->optionsEdit->text().trimmed();
    if (!options.isEmpty())
    {
        args << options.split(" ", Qt::SkipEmptyParts);
    }

    // 输出URL
    args << "-f" << "flv" << ui->urlEdit->text();

    return args;
}