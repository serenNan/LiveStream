#pragma once
#include "FileLog.h"
#include "Logger.h"
#include <cstdint>
#include <json/json.h>
#include <memory>
#include <string>

namespace tmms
{
    namespace base
    {
        struct LogInfo
        {
            LogLevel level;
            std::string path;
            std::string name;
            RotateType rotate_type{kRotateNone};
        };
        using LogInfoPtr = std::shared_ptr<LogInfo>;

        class Config
        {
          public:
            Config() = default;
            ~Config() = default;

            bool LoadConfig(const std::string &file);
            LogInfoPtr &GetLogInfo();

            std::string name_;
            int32_t cpu_start_{0};
            int32_t thread_nums_{1};

          private:
            bool ParseLogInfo(const Json::Value &root);
            LogInfoPtr log_info_;
        };
    } // namespace base
} // namespace tmms