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
        /**
         * @brief 日志配置信息结构体
         * 包含日志级别、路径、文件名和切分类型
         */
        struct LogInfo
        {
            LogLevel level;                      ///< 日志级别
            std::string path;                    ///< 日志文件路径
            std::string name;                    ///< 日志文件名
            RotateType rotate_type{kRotateNone}; ///< 日志切分类型
        };

        /**
         * @brief 服务器信息结构体
         * 包含服务地址、端口、协议类型和传输层协议
         */
        struct ServiceInfo
        {
            std::string addr;      ///< 服务地址，如IP地址或域名
            uint16_t port;         ///< 服务端口号
            std::string protocol;  ///< 应用层协议类型，如"HTTP"、"RTMP"等
            std::string transport; ///< 传输层协议类型，如"TCP"或"UDP"
        };

        using LogInfoPtr = std::shared_ptr<LogInfo>; ///< 日志配置信息智能指针类型
        using ServiceInfoPtr = std::shared_ptr<ServiceInfo>; ///< 服务器信息智能指针类型

        /**
         * @brief 配置类
         * 负责加载和解析配置文件，提供配置信息访问接口
         */
        class Config
        {
          public:
            Config() = default;
            ~Config() = default;

            /**
             * @brief 加载配置文件
             * @param file 配置文件路径
             * @return true 加载成功，false 加载失败
             */
            bool LoadConfig(const std::string &file);

            /**
             * @brief 获取日志配置信息
             * @return 日志配置信息指针
             */
            LogInfoPtr &GetLogInfo();

            /**
             * @brief 获取服务器信息列表
             * @return 服务器信息列表
             */
            const std::vector<ServiceInfoPtr> &GetServiceInfos();

            /**
             * @brief 获取服务器信息
             * @param protocol 协议类型
             * @param transport 传输层协议类型
             * @return 服务器信息指针
             */
            const ServiceInfoPtr &GetServiceInfo(const std::string &protocol,
                                                 const std::string &transport);

            /**
             * @brief 解析服务器信息
             * @param serviceObj 服务器信息JSON对象
             * @return true 解析成功，false 解析失败
             */
            bool ParseServiceInfo(const Json::Value &serviceObj);

            std::string name_;       ///< 配置名称
            int32_t cpu_start_{0};   ///< CPU起始编号
            int32_t thread_nums_{1}; ///< 线程数量

          private:
            /**
             * @brief 解析日志配置信息
             * @param root JSON配置根节点
             * @return true 解析成功，false 解析失败
             */
            bool ParseLogInfo(const Json::Value &root);
            LogInfoPtr log_info_;                  ///< 日志配置信息
            std::vector<ServiceInfoPtr> services_; ///< 服务器信息列表
        };
    } // namespace base
} // namespace tmms