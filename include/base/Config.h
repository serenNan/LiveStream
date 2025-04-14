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
            LogLevel level;      ///< 日志级别
            std::string path;    ///< 日志文件路径
            std::string name;    ///< 日志文件名
            RotateType rotate_type{kRotateNone}; ///< 日志切分类型
        };
        /// @brief 日志配置信息智能指针类型
        using LogInfoPtr = std::shared_ptr<LogInfo>;

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
            LogInfoPtr log_info_;    ///< 日志配置信息
        };
    } // namespace base
} // namespace tmms