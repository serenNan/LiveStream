#include "MainWindow.h"
#include <QApplication>
#include <QCoreApplication>
#pragma comment(lib, "user32.lib")

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

    // 创建并显示主窗口
    MainWindow w;
    w.show();

    // 运行应用程序事件循环
    return a.exec();
}