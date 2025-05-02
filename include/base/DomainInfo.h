#pragma once
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace tmms
{
    namespace base
    {

        class AppInfo;
        using AppInfoPtr = std::shared_ptr<AppInfo>;
        class DomainInfo
        {
          public:
            DomainInfo() = default;
            ~DomainInfo() = default;

            /**
             * @brief 获取域名
             * @return 返回域名的常量引用，防止调用者修改值
             */
            const std::string &DomainName() const;

            /**
             * @brief 获取域类型
             * @return 返回域类型的常量引用
             */
            const std::string &Type() const;

            /**
             * @brief 解析指定文件中的域信息
             * @param file 要解析的配置文件路径
             * @return 解析成功返回true，失败返回false
             * @details 使用Json解析器读取文件内容，提取域名和类型等信息
             */
            bool ParseDomainInfo(const std::string &file);

            /**
             * @brief 通过应用名称获取对应的应用信息
             * @param app_name 应用名称
             * @return 返回对应的AppInfo智能指针对象，如果不存在则返回nullptr
             * @details 线程安全，内部使用互斥锁保护访问
             */
            AppInfoPtr GetAppInfo(const std::string &app_name);

          private:
            std::string name_; ///< 域名
            std::string type_; ///< 域类型
            std::mutex lock_;  ///< 互斥锁，用于保护appinfos_的线程安全访问
            std::unordered_map<std::string, AppInfoPtr>
                appinfos_; ///< 应用信息映射表，key为应用名称
        };
    } // namespace base
} // namespace tmms