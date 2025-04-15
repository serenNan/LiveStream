#include "EventLoop.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"
#include "PipeEvent.h"
#include "TTime.h"
#include <iostream>
using namespace tmms::network;

EventLoopThread eventloop_thread;
std::thread th;

void TestEventLoopThread()
{
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();

    if (loop)
    {
        std::cout << "loop:" << loop << std::endl;
        PipeEventPtr pipe = std::make_shared<PipeEvent>(loop);
        loop->AddEvent(pipe);
        int64_t test = 100;
        pipe->Write((const char *)&test, sizeof(test));

        th = std::thread([&pipe]() {
            while (1)
            {
                int64_t now = tmms::base::TTime::NowMS();
                pipe->Write((const char *)&now, sizeof(now));
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

void TestEventLoopThreadPool()
{
    EventLoopThreadPool pool(2, 0, 2);

    pool.Start();

    std::vector<EventLoop *> loops = pool.GetLoops();
    for (auto &loop : loops)
    {
        std::cout << "loop:" << loop << std::endl;
    }

    EventLoop *loop = pool.GetNextLoop();
    std::cout << "loop:" << loop << std::endl;
    loop = pool.GetNextLoop();
    std::cout << "loop:" << loop << std::endl;
}

int main(int argc, const char **argv)
{
    TestEventLoopThreadPool();
    return 0;
}
