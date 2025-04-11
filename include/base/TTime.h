#pragma once
#include <cstdint>
#include <string>

namespace tmms
{
namespace base
{
/**
 * @brief 时间工具类，提供获取当前时间的各种方法
 */
class TTime
{
  public:
    /**
     * @brief 获取当前UTC时间戳(毫秒)
     * @return 当前时间戳(毫秒)
     */
    static int64_t NowMS();
    
    /**
     * @brief 获取当前UTC时间戳(秒)
     * @return 当前时间戳(秒)
     */
    static int64_t Now();
    
    /**
     * @brief 获取当前时间戳并分解为年月日时分秒
     * @param year 年份
     * @param month 月份(1-12)
     * @param day 日(1-31)
     * @param hour 小时(0-23)
     * @param minute 分钟(0-59)
     * @param second 秒(0-59)
     * @return 当前时间戳(秒)
     */
    static int64_t Now(int &year, int &mouth, int &day, int &hour, int &minute, int &second);
    
    /**
     * @brief 获取当前时间的ISO 8601格式字符串
     * @return ISO 8601格式的时间字符串(如"2023-01-01T12:00:00Z")
     */
    static std::string ISOTime();
};
} // namespace base
} // namespace tmms