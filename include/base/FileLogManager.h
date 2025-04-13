#pragma once

#include "FileLog.h"
#include "NonCopyable.h"
#include "Singleton.h"
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace tmms
{
    namespace base
    {
        
        class FileLogManager : NonCopyable
        {
          public:
            FileLogManager() = default;
            ~FileLogManager() = default;
            void OnCheck();
            FileLogPtr GetFileLog(const std::string &fileName);
            void RemoveFileLog(const FileLogPtr &log);
            void RotateDays(const FileLogPtr &file);
            void RotateHours(const FileLogPtr &file);
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

#define sFileLogManager tmms::base::Singleton<tmms::base::FileLogManager>::Instance()