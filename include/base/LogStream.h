#pragma once
#include "Logger.h"
#include <sstream>
namespace tmms
{
    namespace base
    {
        extern Logger *g_logger; ///< 全局日志记录器指针
        /**
         * @brief 日志流类
         * 提供流式日志记录接口，支持<<操作符
         */
        class LogStream
        {
          public:
            /**
             * @brief 构造函数
             * @param logger 日志记录器指针
             * @param file 源文件名
             * @param line 行号
             * @param level 日志级别
             * @param func 函数名(可选)
             */
            LogStream(Logger *logger, const char *file, int line, LogLevel level,
                      const char *func = nullptr);
            ~LogStream();
            /**
             * @brief 流式日志输出操作符
             * @tparam T 可输出类型
             * @param value 要输出的值
             * @return 日志流引用
             */
            template <class T> LogStream &operator<<(const T &value)
            {
                stream_ << value;
                return *this; // 链式引用
            }

          private:
            std::ostringstream stream_; ///< 输出字符串流
            Logger *logger_{nullptr};   ///< 关联的日志记录器
        };
    } // namespace base
} // namespace tmms

/**
 * @brief 跟踪级别日志宏
 * 当日志级别<=kTrace时输出
 */
#define LOG_TRACE                                                                                  \
    if (g_logger && tmms::base::g_logger->GetLogLevel() <= tmms::base::kTrace)                     \
    tmms::base::LogStream(tmms::base::g_logger, __FILE__, __LINE__, tmms::base::kTrace,            \
                          __FUNCTION__)

/**
 * @brief 调试级别日志宏
 * 当日志级别<=kDebug时输出
 */
#define LOG_DEBUG                                                                                  \
    if (g_logger && tmms::base::g_logger->GetLogLevel() <= tmms::base::kDebug)                     \
    tmms::base::LogStream(tmms::base::g_logger, __FILE__, __LINE__, tmms::base::kDebug,            \
                          __FUNCTION__)

/**
 * @brief 信息级别日志宏
 * 当日志级别<=kInfo时输出
 */
#define LOG_INFO                                                                                   \
    if (g_logger && tmms::base::g_logger->GetLogLevel() <= tmms::base::kInfo)                      \
    tmms::base::LogStream(tmms::base::g_logger, __FILE__, __LINE__, tmms::base::kInfo)

/**
 * @brief 警告级别日志宏
 */
#define LOG_WARN                                                                                   \
    tmms::base::LogStream(tmms::base::g_logger, __FILE__, __LINE__, tmms::base::kWarn, __FUNCTION__)

/**
 * @brief 错误级别日志宏
 */
#define LOG_ERROR                                                                                tmms::base::LogStream(tmms::base::g_logger, __FILE__, __LINE__, tmms::base::kError, __FUNCTION__)