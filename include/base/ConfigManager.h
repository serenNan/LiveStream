#pragma once
#include "Config.h"
#include "NonCopyable.h"
#include <memory>
#include <mutex>
namespace tmms
{
    namespace base
    {
        using ConfigPtr = std::shared_ptr<Config>;
        class ConfigManager : public NonCopyable
        {
          public:
            ConfigManager() = default;
            ~ConfigManager() = default;

            bool LoadConfig(const std::string &file);
            ConfigPtr GetConfig();

          private:
            ConfigPtr config_;
            std::mutex lock_;
        };
    } // namespace base
} // namespace tmms

#define sConfigManager tmms::base::Singleton<tmms::base::ConfigManager>::Instance()
