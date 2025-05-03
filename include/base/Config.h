#pragma once
#include "FileLog.h"
#include "LogStream.h"
#include "Logger.h"
#include "NonCopyable.h"
#include "Singleton.h"
#include "json/json.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

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

        class AppInfo;
        class DomainInfo;
        using LogInfoPtr = std::shared_ptr<LogInfo>; ///< 日志配置信息智能指针类型
        using ServiceInfoPtr = std::shared_ptr<ServiceInfo>; ///< 服务器信息智能指针类型
        using AppInfoPtr = std::shared_ptr<AppInfo>;         ///< 应用信息智能指针类型
        using DomainInfoPtr = std::shared_ptr<DomainInfo>;   ///< 域信息智能指针类型
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

            /**
             * @brief 获取应用信息
             * @param domain 域名
             * @param app 应用名称
             * @return 应用信息指针，如果不存在则返回nullptr
             * @details 通过域名和应用名称查找对应的应用配置信息
             */
            AppInfoPtr GetAppInfo(const std::string &domain, const std::string &app);

            /**
             * @brief 获取域信息
             * @param domain 域名
             * @return 域信息指针，如果不存在则返回nullptr
             * @details 根据域名查找对应的域配置信息
             */
            DomainInfoPtr GetDomainInfo(const std::string &domain);

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

            /**
             * @brief 解析目录配置信息
             * @param root JSON配置根节点
             * @return true 解析成功，false 解析失败
             * @details 从JSON配置中解析域配置文件所在的目录信息
             */
            bool ParseDirectory(const Json::Value &root);

            /**
             * @brief 解析域配置路径
             * @param path 域配置目录路径
             * @return true 解析成功，false 解析失败
             * @details 扫描指定目录下的所有域配置文件并解析
             */
            bool ParseDomainPath(const std::string &path);

            /**
             * @brief 解析单个域配置文件
             * @param file 域配置文件路径
             * @return true 解析成功，false 解析失败
             * @details 从指定的配置文件中解析域信息
             */
            bool ParseDomainFile(const std::string &file);

            LogInfoPtr log_info_;                                        ///< 日志配置信息
            std::vector<ServiceInfoPtr> services_;                       ///< 服务器信息列表
            std::unordered_map<std::string, DomainInfoPtr> domaininfos_; ///< 域信息列表
            std::mutex lock_;                                            ///< 互斥锁
        };
    } // namespace base
} // namespace tmms