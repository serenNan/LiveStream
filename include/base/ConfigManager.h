#pragma once
#include "Config.h"
#include "NonCopyable.h"
#include <memory>
#include <mutex>
namespace tmms
{
    namespace base
    {
        /// @brief 配置指针类型
        using ConfigPtr = std::shared_ptr<Config>;
        
        /**
         * @brief 配置管理类
         * 负责加载和管理配置信息，提供线程安全的配置访问接口（热更新）
         */
        class ConfigManager : public NonCopyable
        {
          public:
            ConfigManager() = default;
            ~ConfigManager() = default;

            /**
             * @brief 加载配置文件
             * @param file 配置文件路径
             * @return true 加载成功，false 加载失败
             */
            bool LoadConfig(const std::string &file);
            
            /**
             * @brief 获取配置信息
             * @return 配置信息指针
             */
            ConfigPtr GetConfig();

          private:
            ConfigPtr config_; ///< 配置信息
            std::mutex lock_;  ///< 互斥锁，保证线程安全
        };
    } // namespace base
} // namespace tmms

/// @brief 全局配置管理器单例宏
#define sConfigManager tmms::base::Singleton<tmms::base::ConfigManager>::Instance()
