#include "EventLoop.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"
#include "PipeEvent.h"
#include "TTime.h"
#include <chrono>
#include <thread>
#include "gtest/gtest.h"
using namespace tmms::network;

// EventLoopThread eventloop_thread;
// std::thread th;

TEST(TestEventLoop, BasicThreadTest)
{
    EventLoopThread eventloop_thread;
    std::thread th;
    
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();
    ASSERT_NE(loop, nullptr);
    
    PipeEventPtr pipe = std::make_shared<PipeEvent>(loop);
    loop->AddEvent(pipe);
    int64_t test = 100;
    pipe->Write((const char *)&test, sizeof(test));
    
    th = std::thread([&pipe]() {
        for(int i = 0; i < 3; i++) {
            int64_t now = tmms::base::TTime::NowMS();
            pipe->Write((const char *)&now, sizeof(now));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    
    th.join();
}

TEST(TestEventLoop, ThreadPoolTest)
{
    EventLoopThreadPool pool(2, 0, 4);
    pool.Start();
    
    EventLoop *loop = pool.GetNextLoop();
    ASSERT_NE(loop, nullptr);
    
    int count = 0;
    loop->RunAfter(1, [&count]() { 
        count++;
        EXPECT_GT(tmms::base::TTime::Now(), 0);
    });
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_EQ(count, 1);
}

TEST(TestEventLoop, TimerTest)
{
    EventLoopThread eventloop_thread;
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();
    ASSERT_NE(loop, nullptr);
    
    loop->RunAfter(1, []() { std::cout << "run after 1s now:" << tmms::base::TTime::Now() << std::endl; });
    loop->RunAfter(5, []() { std::cout << "run after 5s now:" << tmms::base::TTime::Now() << std::endl; });
    loop->RunEvery(1, []() { std::cout << "run every 1s now:" << tmms::base::TTime::Now() << std::endl; });
    loop->RunEvery(5, []() { std::cout << "run every 5s now:" << tmms::base::TTime::Now() << std::endl; });
    
    std::this_thread::sleep_for(std::chrono::seconds(10));
}

