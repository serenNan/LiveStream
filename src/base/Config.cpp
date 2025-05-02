#include "Config.h"
#include "LogStream.h"
#include <json/json.h>
#include <fstream>

using namespace tmms::base;

namespace
{
    static const ServiceInfoPtr service_info_nullptr;
}

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

    if (!ParseServiceInfo(root["service"]))
    {
        LOG_ERROR << "parse service info failed!";
        return false;
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

const std::vector<ServiceInfoPtr> &Config::GetServiceInfos()
{
    // 返回服务信息列表
    return services_;
}

const ServiceInfoPtr &Config::GetServiceInfo(const std::string &protocol, const std::string &transport)
{
    // 遍历服务信息，根据协议和传输层协议找到匹配的服务信息并返回
    for (auto &s : services_)
    {
        if (s->protocol == protocol && s->transport == transport)
        {
            return s;
        }
    }

    // 如果未找到，返回空指针
    return service_info_nullptr;
}

// 解析服务信息，解析成功则将服务信息添加到 services_ 容器中
bool Config::ParseServiceInfo(const Json::Value &serviceObj)
{
    // 如果配置文件中没有 service 段，记录错误并返回 false
    if (serviceObj.isNull())
    {
        LOG_ERROR << " config no service section!";
        return false;
    }

    // 如果 service 段不是数组类型，记录错误并返回 false
    if (!serviceObj.isArray())
    {
        LOG_ERROR << " service section type is not array!";
        return false;
    }

    // 遍历解析的服务信息
    for (auto const &s : serviceObj)
    {
        // 创建 ServiceInfo 的智能指针
        ServiceInfoPtr sinfo = std::make_shared<ServiceInfo>();

        // 解析每个服务的地址、端口、协议和传输层协议
        sinfo->addr = s.get("addr", "0.0.0.0").asString();
        sinfo->port = s.get("port", "0").asInt();
        sinfo->protocol = s.get("protocol", "rtmp").asString();
        sinfo->transport = s.get("transport", "tcp").asString();

        // 记录解析的服务信息
        LOG_INFO << " service info addr : " << sinfo->addr << " port : " << sinfo->port
                 << " protocol : " << sinfo->protocol << " transport : " << sinfo->transport;

        // 将解析后的服务信息添加到 services_ 容器中
        services_.emplace_back(sinfo);
    }

    // 解析成功
    return true;
}