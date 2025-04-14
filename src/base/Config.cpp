#include "Config.h"
#include "LogStream.h"
#include <json/json.h>
#include <fstream>

using namespace tmms::base;

bool Config::LoadConfig(const std::string &file)
{
    LOG_DEBUG << "config file:" << file;
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::ifstream in(file);
    std::string err;
    auto ok = Json::parseFromStream(reader, in, &root, &err);
    if (!ok)
    {
        LOG_ERROR << "config file:" << file
                  << " parse error.err"  << err;
        return false;
    }

    Json::Value nameObj = root["name"];
    if (!nameObj.isNull())
    {
        name_ = nameObj.asString();
    }

    Json::Value cpusObj = root["cpu_start"];
    if (!cpusObj.isNull())
    {
        cpu_start_ = cpusObj.asInt();
    }

    Json::Value threadsObj = root["threads"];
    if (!threadsObj.isNull())
    {
        thread_nums_ = threadsObj.asInt();
    }

    Json::Value logObj = root["log"];
    if (!logObj.isNull())
    {
        ParseLogInfo(logObj);
    }
    return true;
}

bool Config::ParseLogInfo(const Json::Value &root)
{
    log_info_ = std::make_shared<LogInfo>();

    Json::Value levelObj = root["level"];
    if (!levelObj.isNull())
    {
        std::string level = levelObj.asString();
        if (level == "TRACE")
        {
            log_info_->level = kTrace;
        }
        else if (level == "DEBUG")
        {
            log_info_->level = kDebug;
        }
        else if (level == "INFO")
        {
            log_info_->level = kInfo;
        }
        else if (level == "WARN")
        {
            log_info_->level = kWarn;
        }
        else if (level == "ERROR")
        {
            log_info_->level = kError;
        }
    }
    Json::Value rotateObj = root["rotate"];
    if (!rotateObj.isNull())
    {
        std::string rotate = rotateObj.asString();
        if (rotate == "DAY")
        {
            log_info_->rotate_type = kRotateDay;
        }
        else if (rotate == "HOUR")
        {
            log_info_->rotate_type = kRotateHour;
        }
        else if (rotate == "MINUTE")
        {
            log_info_->rotate_type = kRotateMinute;
        }
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
    return true;
}

LogInfoPtr &Config::GetLogInfo()
{
    return log_info_;
}