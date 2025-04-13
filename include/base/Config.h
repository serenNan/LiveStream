#pragma once
#include <cstdint>
#include "json.h"
#include <string>
#include <memory>

namespace tmms
{
    namespace base
    {
        struct LogInfo
        {
            std::string level;
            std::string path;
            std::string name;
        };
        using LogInfoPtr = std::shared_ptr<LogInfo>;
        
        class Config
        {
          public:
            Config() = default;
            ~Config() = default;

            bool LoadConfig(const std::string &file);
            bool ParseLogInfo(const Json::Value &root);
            LogInfoPtr &GetLogInfo();

            std::string name_;
            int32_t cpu_start_{0};
            int32_t thread_num_{1};
          private:
            LogInfoPtr log_info_;
        };
    } // namespace base
} // namespace tmms