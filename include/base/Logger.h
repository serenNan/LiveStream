#pragma once
#include "FileLog.h"
#include "NonCopyable.h"
#include "base/FileLogManager.h"
#include <string>
namespace tmms
{
    namespace base
    {
        /**
         * @brief 日志级别枚举
         * 定义了从跟踪到错误的5种日志级别
         */
        enum LogLevel
        {
            kTrace,           ///< 跟踪级别，最详细的日志信息
            kDebug,           ///< 调试级别，用于开发调试
            kInfo,            ///< 信息级别，常规运行信息
            kWarn,            ///< 警告级别，潜在问题
            kError,           ///< 错误级别，错误事件
            kMaxNumLogLevels, ///< 日志级别总数
        };

        /**
         * @brief 日志记录器类
         * 提供日志级别设置和日志写入功能
         */
        class Logger : public NonCopyable
        {
          public:
            Logger(const FileLogPtr &log);
            ~Logger() = default;

            /**
             * @brief 设置日志级别
             * @param level 要设置的日志级别
             */
            void SetLogLevel(const LogLevel &level);

            /**
             * @brief 获取当前日志级别
             * @return 当前日志级别
             */
            LogLevel GetLogLevel() const;

            /**
             * @brief 写入日志
             * @param message 要记录的日志消息
             */
            void WriteLog(const std::string &message);

          private:
            LogLevel level_{kDebug}; ///< 当前日志级别，默认为调试级别
            FileLogPtr log_;         ///< 日志文件对象指针
        };
    } // namespace base
} // namespace tmms