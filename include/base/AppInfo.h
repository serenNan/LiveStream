#pragma once
#include "json/json.h"
#include <memory>
#include <string>

namespace tmms
{
    namespace base
    {
        class DomainInfo;

        class AppInfo
        {
          public:
            /**
             * @brief 构造函数，创建一个应用信息对象
             * @param d 关联的域信息对象引用
             */
            explicit AppInfo(DomainInfo &d);

            /**
             * @brief 解析应用信息
             * @param root JSON格式的应用配置信息
             * @return 解析成功返回true，失败返回false
             * @details 从JSON对象中提取应用的各项配置参数
             */
            bool ParseAppInfo(Json::Value &root);

            DomainInfo &domain_info_;                 ///< 关联的域信息引用
            std::string domain_name_;                 ///< 域名称
            std::string app_name_;                    ///< 应用名称
            uint32_t max_buffer_{1000};               ///< 最大缓冲区大小
            bool rtmp_support_{false};                ///< 是否支持RTMP协议
            bool flv_support_{false};                 ///< 是否支持FLV格式
            bool hls_support_{false};                 ///< 是否支持HLS协议
            uint32_t content_latency_{3 * 1000};      ///< 内容延迟时间(毫秒)
            uint32_t stream_idle_time_{30 * 1000};    ///< 流空闲超时时间(毫秒)
            uint32_t stream_timeout_time_{30 * 1000}; ///< 流连接超时时间(毫秒)
        };
    } // namespace base
} // namespace tmms