#pragma once

#include <QDialog>
#include <QProcess>
#include <QTimer>

namespace Ui
{
    class PushStreamDialog;
}

/**
 * @brief 推流对话框类，用于配置和启动FFmpeg推流
 */
class PushStreamDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit PushStreamDialog(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~PushStreamDialog();

  private slots:
    /**
     * @brief 选择视频文件按钮点击事件
     */
    void on_selectFileButton_clicked();

    /**
     * @brief 开始/停止推流按钮点击事件
     */
    void on_pushButton_clicked();

    /**
     * @brief 处理FFmpeg进程的标准输出
     */
    void handleProcessOutput();

    /**
     * @brief 处理FFmpeg进程的错误输出
     */
    void handleProcessError();

    /**
     * @brief 处理FFmpeg进程结束
     * @param exitCode 进程退出码
     * @param exitStatus 进程退出状态
     */
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    /**
     * @brief 更新推流时间
     */
    void updateStreamTime();

  private:
    Ui::PushStreamDialog *ui;  ///< UI界面指针
    QProcess *m_ffmpegProcess; ///< FFmpeg进程指针
    QString m_selectedFile;    ///< 选择的文件路径
    bool m_isStreaming;        ///< 是否正在推流
    QTimer *m_timeTimer;       ///< 计时器
    int m_streamSeconds;       ///< 推流时间（秒）

    /**
     * @brief 更新UI状态
     */
    void updateUIState();

    /**
     * @brief 构建FFmpeg命令行参数
     * @return 参数列表
     */
    QStringList buildFFmpegArgs();
};