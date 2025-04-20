#include "EventLoop.h"
#include "Event.h"
#include "NetWork.h"
#include "PipeEvent.h"
#include "TTime.h"
#include <asm-generic/socket.h>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <mutex>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace tmms::network;

static thread_local EventLoop *t_local_eventloop = nullptr; ///< 线程局部存储的事件循环指针
EventLoop::EventLoop() : epoll_fd_(::epoll_create(1024)), epoll_events_(1024)
{
    if (t_local_eventloop)
    {
        NETWORK_ERROR << "EventLoop already exists.";
        exit(-1);
    }
    t_local_eventloop = this;
}

EventLoop::~EventLoop()
{
    Quit();
}

void EventLoop::Loop()
{
    lopping_ = true;
    int64_t timeout = 1000;
    while (lopping_)
    {
        memset(&epoll_events_[0], 0x00, sizeof(struct epoll_event) * epoll_events_.size());
        int ret = ::epoll_wait(epoll_fd_, (struct epoll_event *)&epoll_events_[0],
                               static_cast<int>(epoll_events_.size()), timeout);
        if (ret >= 0)
        {
            for (int i = 0; i < ret; ++i)
            {
                struct epoll_event &ev = epoll_events_[i];
                if (ev.data.fd <= 0)
                {
                    continue;
                }
                auto iter = events_.find(ev.data.fd);
                if (iter == events_.end())
                {
                    continue;
                }
                EventPtr &event = iter->second;
                if (ev.events & EPOLLERR)
                {
                    int error = 0;
                    socklen_t len = sizeof(error);
                    getsockopt(event->Fd(), SOL_SOCKET, SO_ERROR, &error, &len);
                    event->OnError((strerror(error)));
                }
                else if ((ev.events & EPOLLHUP) && !(ev.events & EPOLLIN))
                {
                    event->OnClose();
                }
                else if (ev.events & (EPOLLIN | EPOLLPRI))
                {
                    event->OnRead();
                }
                else if (ev.events & EPOLLOUT)
                {
                    event->OnWrite();
                }
            }
            if (ret == epoll_events_.size())
            {
                // 如果事件数量达到上限，扩大容器
                epoll_events_.resize(epoll_events_.size() * 2);
            }
            RunFunctions();
            int64_t now = tmms::base::TTime::NowMS();
            wheel_.OnTimer(now);
        }
        else if (ret < 0)
        {
            if (errno == EINTR) {
                continue;  // 被信号中断，继续循环
            }
            NETWORK_ERROR << "epoll wait error.error:" << errno << " msg:" << strerror(errno);
            // 其他错误可能需要处理
        }
    }
}

void EventLoop::Quit()
{
    lopping_ = false;
}

void EventLoop::AddEvent(const EventPtr &event)
{
    auto iter = events_.find(event->Fd());
    if (iter != events_.end())
    {
        return;
    }
    event->events_ |= kEventRead;
    events_[event->Fd()] = event;

    struct epoll_event ev;
    memset(&ev, 0x00, sizeof(struct epoll_event));
    ev.events = event->events_;
    ev.data.fd = event->fd_;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, event->fd_, &ev) == -1)
    {
        NETWORK_ERROR << "epoll_ctl add error: " << strerror(errno);
        exit(-1);
    }
}

void EventLoop::DelEvent(const EventPtr &event)
{
    auto iter = events_.find(event->Fd());
    if (iter == events_.end())
    {
        return;
    }
    events_.erase(iter);

    struct epoll_event ev;
    memset(&ev, 0x00, sizeof(ev));
    ev.events = event->events_;
    ev.data.fd = event->Fd();
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, event->fd_, &ev);
}

bool EventLoop::EnableEventReading(const EventPtr &event, bool enable)
{
    auto iter = events_.find(event->Fd());
    if (iter == events_.end())
    {
        NETWORK_ERROR << "event fd:" << event->Fd() << " not exist, enable:" << enable 
                     << ", events size:" << events_.size();
        return false;
    }

    if (enable)
    {
        event->events_ |= kEventRead;
    }
    else
    {
        event->events_ &= ~kEventRead;
    }

    struct epoll_event ev;
    memset(&ev, 0x00, sizeof(struct epoll_event));
    ev.events = event->events_;
    ev.data.fd = event->fd_;
    epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, event->fd_, &ev);
    return true;
}

bool EventLoop::EnableEventWriting(const EventPtr &event, bool enable)
{
    auto iter = events_.find(event->Fd());
    if (iter == events_.end())
    {
        NETWORK_ERROR << "event fd:" << event->Fd() << " not exist";
        return false;
    }

    if (enable)
    {
        event->events_ |= kEventWrite;
    }
    else
    {
        event->events_ &= ~kEventWrite;
    }

    struct epoll_event ev;
    memset(&ev, 0x00, sizeof(struct epoll_event));
    ev.events = event->events_;
    ev.data.fd = event->Fd();
    epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, event->fd_, &ev);
    return true;
}

void EventLoop::AssertInLoopThread()
{
    if (!IsInLoopThread())
    {
        NETWORK_ERROR << "EventLoop is not in loop thread.";
        exit(-1);
    }
}

bool EventLoop::IsInLoopThread() const
{
    return t_local_eventloop == this;
}

void EventLoop::RunInLoop(const Func &func)
{
    if (IsInLoopThread())
    {
        func();
    }
    else
    {
        std::lock_guard<std::mutex> lk(lock_);
        functions_.push(func);

        WakeUp();
    }
}

void EventLoop::RunInLoop(Func &&func)
{
    if (IsInLoopThread())
    {
        func();
    }
    else
    {
        std::lock_guard<std::mutex> lk(lock_);
        functions_.push(std::move(func));

        WakeUp();
    }
}

void EventLoop::RunFunctions()
{
    std::lock_guard<std::mutex> lk(lock_);
    while (!functions_.empty())
    {
        auto &func = functions_.front();
        func();
        functions_.pop();
    }
}

void EventLoop::WakeUp()
{
    if (!pipe_event_)
    {
        pipe_event_ = std::make_shared<PipeEvent>(this);
        AddEvent(pipe_event_);
    }
    int64_t tmp = 1;
    pipe_event_->Write((const char *)&tmp, sizeof(tmp));
}

void EventLoop::InsertEntry(uint32_t delay, EntryPtr entryPty)
{
    if (IsInLoopThread())
    {
        wheel_.InsertEntry(delay, entryPty);
    }
    else
    {
        RunInLoop([this, delay, entryPty]() { wheel_.InsertEntry(delay, entryPty); });
    }
}

void EventLoop::RunAfter(double delay, const Func &cb)
{
    if (IsInLoopThread())
    {
        wheel_.RunAfter(delay, cb);
    }
    else
    {
        RunInLoop([this, delay, cb]() { wheel_.RunAfter(delay, cb); });
    }
}

void EventLoop::RunAfter(double delay, Func &&cb)
{
    if (IsInLoopThread())
    {
        wheel_.RunAfter(delay, cb);
    }
    else
    {
        RunInLoop([this, delay, cb]() { wheel_.RunAfter(delay, cb); });
    }
}

void EventLoop::RunEvery(double interval, const Func &cb)
{
    if (IsInLoopThread())
    {
        wheel_.RunEvery(interval, cb);
    }
    else
    {
        RunInLoop([this, interval, cb]() { wheel_.RunEvery(interval, cb); });
    }
}

void EventLoop::RunEvery(double interval, Func &&cb)
{
    if (IsInLoopThread())
    {
        wheel_.RunEvery(interval, cb);
    }
    else
    {
        RunInLoop([this, interval, cb]() { wheel_.RunEvery(interval, cb); });
    }
}