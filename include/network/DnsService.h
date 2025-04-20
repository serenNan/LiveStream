#pragma once
#include "SocketOpt.h"
#include "base/NonCopyable.h"
#include "base/Singleton.h"
#include "network/base/InetAddress.h"
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace tmms
{
    namespace network
    {
        using InetAddressPtr = std::shared_ptr<InetAddress>;
        /**
         * DNS服务类，负责管理主机名到IP地址的解析和缓存
         * 提供定时更新和并发安全访问功能
         */
        class DnsService : public base::NonCopyable
        {
          public:
            DnsService() = default;

            /**
             * 添加需要解析的主机名
             * @param host 主机名字符串
             */
            void AddHost(const std::string &host);

            /**
             * 获取指定主机的IP地址(按索引)
             * @param host 主机名字符串
             * @param index IP地址索引
             * @return 共享指针指向的InetAddress对象
             */
            InetAddressPtr GetHostAddress(const std::string &host, int index);

            /**
             * 获取指定主机的所有IP地址
             * @param host 主机名字符串
             * @return IP地址列表
             */
            std::vector<InetAddressPtr> GetHostAddress(const std::string &host);

            /**
             * 更新主机的IP地址列表
             * @param host 主机名字符串
             * @param list 新的IP地址列表
             */
            void UpdateHost(const std::string &host, std::vector<InetAddressPtr> &list);

            /**
             * 获取所有主机及其IP地址映射
             * @return 主机到IP地址列表的映射
             */
            std::unordered_map<std::string, std::vector<InetAddressPtr>> GetHosts();

            /**
             * 设置DNS服务参数
             * @param interval 更新间隔(毫秒)
             * @param sleep 重试间隔(毫秒)
             * @param retry 重试次数
             */
            void SetDnsServiceParam(int32_t interval, int32_t sleep, int32_t retry);

            /**
             * 启动DNS服务线程
             */
            void Start();

            /**
             * 停止DNS服务线程
             */
            void Stop();

            /**
             * 工作线程函数
             */
            void OnWork();

            /**
             * 静态方法：解析主机名获取IP地址
             * @param host 主机名字符串
             * @param list 输出参数，存储解析结果
             */
            static void GetHostInfo(const std::string &host, std::vector<InetAddressPtr> &list);

            ~DnsService();

          private:
            std::thread thread_;  ///< DNS服务工作线程
            bool running_{false}; ///< 服务运行状态标志
            std::mutex lock_;     ///< 保护hosts_info_的互斥锁
            std::unordered_map<std::string, std::vector<InetAddressPtr>>
                hosts_info_;               ///< 主机名到IP地址列表的映射缓存
            int32_t retry_{3};             ///< 解析失败重试次数
            int32_t sleep_{200};           ///< 重试间隔时间(毫秒)
            int32_t interval_{180 * 1000}; ///< 定时更新间隔(毫秒)
        };
#define sDnsService tmms::base::Singleton<tmms::network::DnsService>::Instance()
    } // namespace network
} // namespace tmms
