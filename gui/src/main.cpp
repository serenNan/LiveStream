#include "MainWindow.h"
#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QStyleFactory>
#include <QTextStream>
#pragma comment(lib, "user32.lib")

/**
 * @brief 加载样式表
 * @param app QApplication对象
 * @return 是否成功加载
 */
bool loadStyleSheet(QApplication &app)
{
    QFile file(":/style.qss");
    if (!file.exists())
    {
        file.setFileName("./style.qss");
    }
    if (!file.exists())
    {
        file.setFileName("../gui/src/style.qss");
    }

    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream stream(&file);
        app.setStyleSheet(stream.readAll());
        file.close();
        return true;
    }

    return false;
}

/**
 * @brief 程序入口函数
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 程序退出代码
 */
int main(int argc, char *argv[])
{
    // 创建Qt应用程序
    QApplication a(argc, argv);

    // 设置应用程序属性
    QCoreApplication::setApplicationName("LiveStreamingGUI");
    QCoreApplication::setOrganizationName("TMMS");

    // 应用Fusion风格
    a.setStyle(QStyleFactory::create("Fusion"));

    // 加载样式表
    if (!loadStyleSheet(a))
    {
        qWarning() << "Failed to load style sheet!";
    }

    // 创建并显示主窗口
    MainWindow w;
    w.show();

    // 运行应用程序事件循环
    return a.exec();
}