#include "TTime.h"
#include <iostream>
using namespace tmms;
using namespace base;

int main() {
    // 输出毫秒时间戳
    int64_t msTime = TTime::NowMS();
    std::cout << "当前时间戳(ms): " << msTime << std::endl;
    
    // 输出秒时间戳
    int64_t secTime = TTime::Now();
    std::cout << "当前时间戳(s): " << secTime << std::endl;
    
    // 输出分解时间
    int year, month, day, hour, minute, second;
    TTime::Now(year, month, day, hour, minute, second);
    std::cout << "当前时间: " << year << "-" << month << "-" << day << " " 
              << hour << ":" << minute << ":" << second << std::endl;
    
    // 输出ISO8601格式时间
    std::string isoTime = TTime::ISOTime();
    std::cout << "ISO8601格式时间: " << isoTime << std::endl;
    
    return 0;
}