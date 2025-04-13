#include "Config.h"
#include "LogStream.h"
#include "json.h"
#include "value.h"
#include <fstream>

using namespace tmms::base;

bool Config::LoadConfig(const std::string &file)
{
    LOG_DEBUG << "config file:" <<file;
    std::ifstream config_file(file);
    if (!config_file.is_open())
        return false;

    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errs;

    if (!Json::parseFromStream(builder, config_file, &root, &errs))
        return false;

    if (!ParseLogInfo(root))
        return false;

    return true;
}

bool Config::ParseLogInfo(const Json::Value &root)
{
    log_info_ = std::make_shared<LogInfo>();

    Json::Value levelObj = root["level"];
    if (!levelObj.isNull())
    {
        log_info_->level = levelObj.asString();
    }
    Json::Value pathObj = root["path"];
    if (!pathObj.isNull())
    {
        log_info_->path = pathObj.asString();
    }
    Json::Value nameObj = root["name"];
    if (!nameObj.isNull())
    {
        log_info_->name = nameObj.asString();
    }
}

LogInfoPtr &Config::GetLogInfo()
{
    return log_info_;
}