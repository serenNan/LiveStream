#include "ConfigManager.h"
#include "LogStream.h"

using namespace tmms::base;

bool ConfigManager::LoadConfig(const std::string &file)
{
    LOG_DEBUG << "config file:" << file;
    ConfigPtr config = std::make_shared<Config>();
    if (config->LoadConfig(file))
    {
        std::lock_guard<std::mutex> lock(lock_);
        config_ = config;
        return true;
    }
    return false;
}

ConfigPtr ConfigManager::GetConfig()
{
    std::lock_guard<std::mutex> lock(lock_);
    return config_;
}