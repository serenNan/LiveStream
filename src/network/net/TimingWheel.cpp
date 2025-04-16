#include "TimingWheel.h"
#include "NetWork.h"
#include <iostream>
using namespace tmms::network;

TimingWheel::TimingWheel() : wheels_(4)
{
    wheels_[kTimingTypeSecond].resize(60);
    wheels_[kTimingTypeMinute].resize(60);
    wheels_[kTimingTypeHour].resize(24);
    wheels_[kTimingTypeDay].resize(30);
}

void TimingWheel::InsertSecondEntry(uint32_t delay, EntryPtr entryPty)
{
    wheels_[kTimingTypeSecond][delay - 1].emplace(entryPty);
}

void TimingWheel::InsertMinuteEntry(uint32_t delay, EntryPtr entryPty)
{
    auto minute = delay / kTimingMinute;
    auto second = delay % kTimingMinute;
    CallbackEntryPtr newEntryPtr = std::make_shared<CallbackEntry>(
        [this, second, entryPty]() { InsertEntry(second, entryPty); });
    wheels_[kTimingTypeMinute][minute - 1].emplace(entryPty);
}

void TimingWheel::InsertHourEntry(uint32_t delay, EntryPtr entryPty)
{
    auto hour = delay / kTimingHour;
    auto second = delay % kTimingHour;
    CallbackEntryPtr newEntryPtr = std::make_shared<CallbackEntry>(
        [this, second, entryPty]() { InsertEntry(second, entryPty); });
    wheels_[kTimingTypeHour][hour - 1].emplace(entryPty);
}

void TimingWheel::InsertDayEntry(uint32_t delay, EntryPtr entryPty)
{
    auto day = delay / kTimingDay;
    auto second = delay % kTimingDay;
    CallbackEntryPtr newEntryPtr = std::make_shared<CallbackEntry>(
        [this, second, entryPty]() { InsertEntry(second, entryPty); });
    wheels_[kTimingTypeDay][day - 1].emplace(entryPty);
}

TimingWheel::~TimingWheel()
{
}

void TimingWheel::InsertEntry(uint32_t delay, EntryPtr entryPty)
{
    if (delay <= 0)
    {
        entryPty.reset();
    }
    if (delay < kTimingMinute)
    {
        InsertSecondEntry(delay, entryPty);
    }
    else if (delay < kTimingHour)
    {
        InsertMinuteEntry(delay, entryPty);
    }
    else if (delay < kTimingDay)
    {
        InsertHourEntry(delay, entryPty);
    }
    else
    {
        auto day = delay / kTimingDay;
        if (day > 30)
        {
            NETWORK_ERROR << "It is not support > 30 days";
        }
        InsertDayEntry(delay, entryPty);
    }
}

void TimingWheel::OnTimer(int64_t now)
{
    if (last_ts_ == 0)
    {
        last_ts_ = now;
    }

    if (now - last_ts_ < 1000)
    {
        return;
    }
    last_ts_ = now;
    ++tick_;
    PopUp(wheels_[kTimingTypeSecond]);
    if (tick_ % kTimingMinute == 0)
    {
        PopUp(wheels_[kTimingTypeMinute]);
    }
    else if (tick_ % kTimingHour == 0)
    {
        PopUp(wheels_[kTimingTypeHour]);
    }
    else if (tick_ % kTimingDay == 0)
    {
        PopUp(wheels_[kTimingTypeDay]);
    }
}

void TimingWheel::PopUp(Wheel &bq)
{
    WheelEntry tmp;
    bq.front().swap(tmp);
    bq.front().clear();
    bq.pop_front();
    bq.push_back(WheelEntry());
}

void TimingWheel::RunAfter(double delay, const Func &cb)
{
    CallbackEntryPtr cbEntry = std::make_shared<CallbackEntry>([cb]() { cb(); });
    InsertEntry(delay, cbEntry);
}

void TimingWheel::RunAfter(double delay, Func &&cb)
{
    CallbackEntryPtr cbEntry = std::make_shared<CallbackEntry>([cb]() { cb(); });
    InsertEntry(delay, cbEntry);
}

void TimingWheel::RunEvery(double interval, const Func &cb)
{
    CallbackEntryPtr cbEntry = std::make_shared<CallbackEntry>([this,cb, interval]() {
        cb();
        RunEvery(interval, cb);
        });
    InsertEntry(interval, cbEntry);
}

void TimingWheel::RunEvery(double interval, Func &&cb)
{
    CallbackEntryPtr cbEntry = std::make_shared<CallbackEntry>([this, cb, interval]() {
        cb();
        RunEvery(interval, cb);
    });
    InsertEntry(interval, cbEntry);
}
