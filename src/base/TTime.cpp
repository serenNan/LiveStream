#include "base/TTime.h"
#include <chrono>
#include <cstdint>
using namespace tmms::base;

// 表示当前UTC时间，单位是毫秒
int64_t TTime::NowMS()
{
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
}

// 表示当前的UTC时间，单位是秒
int64_t TTime::Now()
{
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
}

// 返回当前时间戳并填充年、月、日等时间参数
int64_t TTime::Now(int &year, int &month, int &day, int &hour, int &minute, int &second)
{
    auto now = std::chrono::system_clock::now();
    time_t time = std::chrono::system_clock::to_time_t(now);
    tm tm_time;
    localtime_r(&time, &tm_time);
    
    year = tm_time.tm_year + 1900;
    month = tm_time.tm_mon + 1;
    day = tm_time.tm_mday;
    hour = tm_time.tm_hour;
    minute = tm_time.tm_min;
    second = tm_time.tm_sec;
    
    return std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
}

std::string TTime::ISOTime()
{
    auto now = std::chrono::system_clock::now();
    time_t time = std::chrono::system_clock::to_time_t(now);
    tm tm_time;
    localtime_r(&time, &tm_time);

    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tm_time);
    return std::string(buffer);
}