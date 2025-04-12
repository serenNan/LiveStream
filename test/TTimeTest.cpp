#include "base/TTime.h"
#include <gtest/gtest.h>
using namespace tmms::base;

TEST(TTimeTest, NowMS_ReturnsCurrentTime) {
    int64_t time1 = TTime::NowMS();
    int64_t time2 = TTime::NowMS();
    std::cout << "当前时间戳(ms): " << time1 << std::endl;
    EXPECT_GE(time2, time1);
}

TEST(TTimeTest, Now_ReturnsCurrentTime) {
    int64_t time1 = TTime::Now();
    int64_t time2 = TTime::Now();
    std::cout << "当前时间戳(s): " << time1 << std::endl;
    EXPECT_GE(time2, time1);
}

TEST(TTimeTest, NowWithParams_ReturnsCurrentTimeAndSetsParams) {
    int year, month, day, hour, minute, second;
    int64_t time = TTime::Now(year, month, day, hour, minute, second);
    
    std::cout << "当前时间: " << year << "-" << month << "-" << day << " " 
              << hour << ":" << minute << ":" << second << std::endl;
    
    EXPECT_GT(year, 2000);
    EXPECT_GE(month, 1);
    EXPECT_LE(month, 12);
    EXPECT_GE(day, 1);
    EXPECT_LE(day, 31);
    EXPECT_GE(hour, 0);
    EXPECT_LE(hour, 23);
    EXPECT_GE(minute, 0);
    EXPECT_LE(minute, 59);
    EXPECT_GE(second, 0);
    EXPECT_LE(second, 59);
}

TEST(TTimeTest, ISOTime_ReturnsValidISO8601Format) {
    std::string isoTime = TTime::ISOTime();
    std::cout << "ISO8601格式时间: " << isoTime << std::endl;
    EXPECT_EQ(isoTime.length(), 20);
    EXPECT_EQ(isoTime[19], 'Z');
}
