#pragma once

#include "FileLog.h"
#include "NonCopyable.h"
#include <mutex>
#include <string>
#include <unordered_map>

namespace tmms
{
    namespace base
    {
        
        /**
         * @brief 文件日志管理器类，管理多个日志文件实例
         * 
         * 提供日志文件实例的获取、移除和切分功能
         */ 
        class FileLogManager : NonCopyable
        {
          public:
            FileLogManager() = default;
            ~FileLogManager() = default;
            /**
             * @brief 定期检查日志文件状态
             */
            void OnCheck();
            /**
             * @brief 获取指定文件名的日志实例
             * @param fileName 日志文件名
             * @return FileLogPtr 日志文件智能指针
             */
            FileLogPtr GetFileLog(const std::string &fileName);
            /**
             * @brief 移除日志实例
             * @param log 要移除的日志实例
             */
            void RemoveFileLog(const FileLogPtr &log);
            /**
             * @brief 按天切分日志文件
             * @param file 要切分的日志文件
             */
            void RotateDays(const FileLogPtr &file);
            /**
             * @brief 按小时切分日志文件
             * @param file 要切分的日志文件
             */
            void RotateHours(const FileLogPtr &file);
            /**
             * @brief 按分钟切分日志文件
             * @param file 要切分的日志文件
             */
            void RotateMinutes(const FileLogPtr &file);

          private:
            std::unordered_map<std::string, FileLogPtr> logs_;
            std::mutex lock_;
            int last_year_{-1};
            int last_month_{-1};
            int last_day_{-1};
            int last_hour_{-1};
            int last_minute_{-1};
        };
    } // namespace base
} // namespace tmms

/**
 * @brief 文件日志管理器单例宏
 */
#define sFileLogManager tmms::base::Singleton<tmms::base::FileLogManager>::Instance()