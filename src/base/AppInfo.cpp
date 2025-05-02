#include "AppInfo.h"
#include "DomainInfo.h"
#include "base/LogStream.h"

using namespace tmms::base;

AppInfo::AppInfo(DomainInfo &d) : domain_info_(d) // 初始化列表
{
}

// 解析 JSON 数据的函数，root 是 JSON 对象的根节点
bool AppInfo::ParseAppInfo(Json::Value &root)
{
    // 从 JSON 对象中获取 "name" 字段，如果存在，将其值赋给 app_name 成员变量
    Json::Value nameObj = root["name"];
    if (!nameObj.isNull())
    {
        app_name_ = nameObj.asString();
    }

    // 从 JSON 对象中获取 "max_buffer" 字段，如果存在，将其值赋给 max_buffer 成员变量
    Json::Value mbObj = root["max_buffer"];
    if (!mbObj.isNull())
    {
        max_buffer_ = mbObj.asUInt();
    }

    // 从 JSON 对象中获取 "hls_support" 字段，如果存在并且值为 "on"，将 hls_support_ 设置为 true
    Json::Value hlsObj = root["hls_support"];
    if (!hlsObj.isNull())
    {
        hls_support_ = hlsObj.asString() == "on";
    }

    // 从 JSON 对象中获取 "flv_support" 字段，如果存在并且值为 "on"，将 flv_support_ 设置为 true
    Json::Value flvObj = root["flv_support"];
    if (!flvObj.isNull())
    {
        flv_support_ = flvObj.asString() == "on";
    }

    // 从 JSON 对象中获取 "rtmp_support" 字段，如果存在并且值为 "on"，将 rtmp_support_ 设置为 true
    Json::Value rtmpObj = root["rtmp_support"];
    if (!rtmpObj.isNull())
    {
        rtmp_support_ = rtmpObj.asString() == "on";
    }

    // 从 JSON 对象中获取 "content_latency" 字段，如果存在，将其值（秒）乘以 1000 并赋给
    // content_latency，单位转换为毫秒
    Json::Value clObj = root["content_latency"];
    if (!clObj.isNull())
    {
        content_latency_ = clObj.asUInt() * 1000;
    }

    // 从 JSON 对象中获取 "stream_idle_time" 字段，如果存在，将其值赋给 stream_idle_time，单位为毫秒
    Json::Value sitObj = root["stream_idle_time"];
    if (!sitObj.isNull())
    {
        stream_idle_time_ = sitObj.asUInt();
    }

    // 从 JSON 对象中获取 "stream_timeout_time" 字段，如果存在，将其值赋给
    // stream_timeout_time，单位为毫秒
    Json::Value sttObj = root["stream_timeout_time"];
    if (!sttObj.isNull())
    {
        stream_timeout_time_ = sttObj.asUInt();
    }

    // 输出日志，显示应用程序的相关信息
    LOG_INFO << " app name : " << app_name_ << " max_buffer : " << max_buffer_
             << " content_latency : " << content_latency_
             << " stream_idle_time : " << stream_idle_time_
             << " stream_timeout_time : " << stream_timeout_time_
             << " rtmp_support : " << rtmp_support_ << " flv_support : " << flv_support_
             << " hls_support : " << hls_support_;

    // 返回 true 表示解析成功
    return true;
}