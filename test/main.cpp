#include "TTime.h"
#include <iostream>
using namespace tmms;
using namespace base;

int main()
{
    // 测试NowMS()
    int64_t time1 = TTime::NowMS();
    int64_t time2 = TTime::NowMS();
    std::cout << "NowMS()测试: time1=" << time1 << ", time2=" << time2
              << ", 结果=" << (time2 >= time1 ? "通过" : "失败") << std::endl;

    // 测试Now()
    time1 = TTime::Now();
    time2 = TTime::Now();
    std::cout << "Now()测试: time1=" << time1 << ", time2=" << time2
              << ", 结果=" << (time2 >= time1 ? "通过" : "失败") << std::endl;

    // 测试Now(year, month...)
    int year, month, day, hour, minute, second;
    int64_t time = TTime::Now(year, month, day, hour, minute, second);
    std::cout << "Now(year...)测试: " << year << "-" << month << "-" << day << " " << hour << ":"
              << minute << ":" << second << ", 结果="
              << ((year > 2000 && month >= 1 && month <= 12 && day >= 1 && day <= 31 && hour >= 0 &&
                   hour <= 23 && minute >= 0 && minute <= 59 && second >= 0 && second <= 59)
                      ? "通过"
                      : "失败")
              << std::endl;

    // 测试ISOTime()
    std::string isoTime = TTime::ISOTime();
    std::cout << "ISOTime()测试: " << isoTime << ", 长度=" << isoTime.length()
              << ", 结果=" << ((isoTime.length() == 20 && isoTime[19] == 'Z') ? "通过" : "失败")
              << std::endl;

    return 0;
}