#include "network/net/EventLoop.h"
#include "network/net/EventLoopThreadPool.h"
#include <iostream>
#include <thread>

using namespace tmms::network;

// 简单的测试任务
void test_task(int task_id)
{
    std::cout << "任务 " << task_id << " 开始执行，线程ID: " << std::this_thread::get_id()
              << std::endl;

    // 模拟工作负载
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::cout << "任务 " << task_id << " 执行完成" << std::endl;
}

int main()
{
    std::cout << "主线程ID: " << std::this_thread::get_id() << std::endl;

    // 1. 创建线程池（2个线程）
    EventLoopThreadPool pool(2);

    // 2. 启动线程池
    std::cout << "启动线程池..." << std::endl;
    pool.Start();

    // 等待线程初始化
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 3. 提交5个简单任务
    std::cout << "\n提交5个任务到线程池..." << std::endl;
    for (int i = 0; i < 5; i++)
    {
        EventLoop *loop = pool.GetNextLoop();
        loop->RunInLoop([i]() { test_task(i); });
        std::cout << "任务 " << i << " 已提交" << std::endl;
    }

    // 4. 等待任务完成
    std::cout << "\n等待任务完成..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "\n测试完成！" << std::endl;
    return 0;
}