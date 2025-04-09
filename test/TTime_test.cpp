#include "TTime.h"
#include <gtest/gtest.h>
using namespace tmms;
using namespace base;

TEST(TTimeTest, NowMS_ReturnsCurrentTime) {
    int64_t time1 = TTime::NowMS();
    int64_t time2 = TTime::NowMS();
    EXPECT_GE(time2, time1);
}

TEST(TTimeTest, Now_ReturnsCurrentTime) {
    int64_t time1 = TTime::Now();
    int64_t time2 = TTime::Now();
    EXPECT_GE(time2, time1);
}

TEST(TTimeTest, NowWithParams_ReturnsCurrentTimeAndSetsParams) {
    int year, month, day, hour, minute, second;
    int64_t time = TTime::Now(year, month, day, hour, minute, second);
    
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
    EXPECT_EQ(isoTime.length(), 20);
    EXPECT_EQ(isoTime[19], 'Z');
}