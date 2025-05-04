#pragma once
#include "base/NonCopyable.h"
#include "base/Singleton.h"
#include "base/Task.h"
#include "base/TaskManager.h"
#include "mmedia/rtmp/RtmpHandler.h"
#include "network/TcpServer.h"
#include "network/net/Connection.h"
#include "network/net/EventLoopThreadPool.h"
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace tmms
{
    namespace live
    {
        using namespace tmms::network;
        using namespace tmms::base;
        using namespace tmms::mm;

        /**
         * @brief 前向声明Session类
         */
        class Session;

        /**
         * @brief SessionPtr是Session的共享指针类型别名
         */
        using SessionPtr = std::shared_ptr<Session>;

        /**
         * @brief 直播服务类，处理RTMP协议的会话服务
         *
         * LiveService类继承自RtmpHandler，负责管理直播会话、处理直播流的发布和播放请求
         */
        class LiveService : public RtmpHandler
        {
          public:
            /**
             * @brief 默认构造函数
             */
            LiveService() = default;

            /**
             * @brief 创建一个新的会话
             * @param session_name 会话名称
             * @return 新创建的会话的智能指针，如果创建失败则返回空指针
             */
            SessionPtr CreateSession(const std::string &session_name);

            /**
             * @brief 查找已有的会话
             * @param session_name 会话名称
             * @return 找到的会话的智能指针，如果未找到则返回空指针
             */
            SessionPtr FindSession(const std::string &session_name);

            /**
             * @brief 关闭指定名称的会话
             * @param session_name 要关闭的会话名称
             * @return 是否成功关闭会话
             */
            bool CloseSession(const std::string &session_name);

            /**
             * @brief 定时器触发的回调函数
             * @param t 定时任务指针
             */
            void OnTimer(const TaskPtr &t);

            /**
             * @brief 当有新的TCP连接时的回调函数
             * @param conn TCP连接指针
             */
            void OnNewConnection(const TcpConnectionPtr &conn) override;

            /**
             * @brief 当TCP连接销毁时的回调函数
             * @param conn 被销毁的TCP连接指针
             */
            void OnConnectionDestroy(const TcpConnectionPtr &conn) override;

            /**
             * @brief 当连接激活时的回调函数
             * @param conn 被激活的连接指针
             */
            void OnActive(const ConnectionPtr &conn) override;

            /**
             * @brief 处理播放请求
             * @param conn TCP连接指针
             * @param session_name 会话名称
             * @param param 附加参数
             * @return 是否成功处理播放请求
             */
            bool OnPlay(const TcpConnectionPtr &conn, const std::string &session_name,
                        const std::string &param) override;

            /**
             * @brief 处理发布请求
             * @param conn TCP连接指针
             * @param session_name 会话名称
             * @param param 附加参数
             * @return 是否成功处理发布请求
             */
            bool OnPublish(const TcpConnectionPtr &conn, const std::string &session_name,
                           const std::string &param) override;

            /**
             * @brief 处理接收的数据(右值引用版本)
             * @param conn TCP连接指针
             * @param data 数据包的右值引用
             */
            void OnRecv(const TcpConnectionPtr &conn, PacketPtr &&data) override;

            /**
             * @brief 处理接收的数据(左值引用版本)
             * @param conn TCP连接指针
             * @param data 数据包的引用
             */
            void OnRecv(const TcpConnectionPtr &conn, const PacketPtr &data) override {};

            /**
             * @brief 启动直播服务
             */
            void Start();

            /**
             * @brief 停止直播服务
             */
            void Stop();

            /**
             * @brief 获取下一个事件循环
             * @return 事件循环指针
             */
            EventLoop *GetNextLoop();

            /**
             * @brief 默认析构函数
             */
            ~LiveService() = default;

          private:
            EventLoopThreadPool *pool_{nullptr}; ///< 事件循环线程池，用于管理多个事件循环
            std::vector<TcpServer *> servers_; ///< 保存所有的TCP服务器实例
            std::mutex lock_;                  ///< 互斥锁，用于保护共享资源
            std::unordered_map<std::string, SessionPtr>
                sessions_; ///< 会话表，键为会话名称，值为会话指针
        };

/**
 * @brief 定义宏sLiveService，用于获取LiveService的单例实例
 */
#define sLiveService lss::base::Singleton<lss::live::LiveService>::Instance()
    } // namespace live
} // namespace tmms