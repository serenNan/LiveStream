#include "base/TaskManager.h"
#include "base/TTime.h"
#include "base/Task.h"
#include "gtest/gtest.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

using namespace tmms::base;

void TestTask()
{
    TaskPtr task1 = std::make_shared<Task>(
        [](const TaskPtr &task) { std::cout << "TestTask1 " << " now:" << TTime::ISOTime()<<std::endl; }, 1000);
    TaskPtr task2 = std::make_shared<Task>(
        [](const TaskPtr &task) {
            std::cout << "TestTask2" << "now:" << TTime::ISOTime() << std::endl;
            task->Restart();
        },
        1000);
    TaskPtr task3 = std::make_shared<Task>(
        [](const TaskPtr &task) {
            std::cout << "TestTask3" << "now:" << TTime::ISOTime() << std::endl;
            task->Restart();
        },
        500);
    TaskPtr task4 = std::make_shared<Task>(
        [](const TaskPtr &task) {
            std::cout << "TestTask4" << "now:" << TTime::ISOTime() << std::endl;
            task->Restart();
        },
        30000);
    sTaskManager->Add(task1);
    sTaskManager->Add(task2);
    sTaskManager->Add(task3);
    sTaskManager->Add(task4);
}

TEST(TestTaskManager, BasicFunctionality)
{
    TestTask();
    for (int i = 0; i < 10; ++i) 
    {
        sTaskManager->OnWork();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}