#pragma once
#include "base/AppInfo.h"
#include "network/net/Connection.h"
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>

namespace tmms
{
    namespace live
    {
        using namespace tmms::network;
        using namespace tmms::base;

        using std::string;

        /**
         * @brief AppInfoPtr是AppInfo的共享指针类型别名
         */
        using AppInfoPtr = std::shared_ptr<AppInfo>;

        /**
         * @brief 前向声明Session类
         */
        class Session;

        /**
         * @brief SessionPtr是Session的共享指针类型别名
         */
        using SessionPtr = std::shared_ptr<Session>;

        /**
         * @brief 用户类型枚举，定义了不同的用户业务类型
         */
        enum class UserType
        {
            kUserTypePublishRtmp = 0, ///< 发布RTMP类型
            kUserTypePublishMpegts,   ///< 发布MPEG-TS类型
            kUserTypePublishPav,      ///< 发布PAV类型
            kUserTypePublishWebRtc,   ///< 发布WebRTC类型
            kUserTypePlayerPav,       ///< 播放PAV类型
            kUserTypePlayerFlv,       ///< 播放FLV类型
            kUserTypePlayerHls,       ///< 播放HLS类型
            kUserTypePlayerRtmp,      ///< 播放RTMP类型
            kUserTypePlayerWebRTC,    ///< 播放WebRTC类型
            kUserTypeUnknowed = 255   ///< 未知用户类型
        };

        /**
         * @brief 用户协议枚举，定义了不同的网络协议类型
         */
        enum class UserProtocol
        {
            kUserProtocolHttp = 0,      ///< HTTP协议
            kUserProtocolHttps,         ///< HTTPS协议
            kUserProtocolQuic,          ///< QUIC协议
            kUserProtocolRtsp,          ///< RTSP协议
            kUserProtocolWebRTC,        ///< WebRTC协议
            kUserProtocolUdp,           ///< UDP协议
            kUserProtocolUnknowed = 255 ///< 未知协议
        };

        /**
         * @brief 前向声明Stream类
         */
        class Stream;

        /**
         * @brief StreamPtr是Stream的共享指针类型别名
         */
        using StreamPtr = std::shared_ptr<Stream>;

        /**
         * @brief 用户类，表示直播系统中的一个用户实体
         *
         * 用户可以是推流者或者播放者，支持多种协议
         */
        class User : public std::enable_shared_from_this<User>
        {
          public:
            /**
             * @brief 让Session类成为友元类，允许Session访问User的私有成员
             */
            friend class Session;

            /**
             * @brief 构造函数
             * @param ptr 连接指针
             * @param stream 流指针
             * @param s 会话指针
             */
            explicit User(const ConnectionPtr &ptr, const StreamPtr &stream, const SessionPtr &s);

            /**
             * @brief 获取域名
             * @return 域名字符串的常量引用
             */
            const string &DomainName() const;

            /**
             * @brief 设置域名
             * @param domain 域名字符串
             */
            void SetDomainName(const string &domain);

            /**
             * @brief 获取应用名称
             * @return 应用名称字符串的常量引用
             */
            const string &AppName() const;

            /**
             * @brief 设置应用名称
             * @param app 应用名称字符串
             */
            void SetAppName(const string &app);

            /**
             * @brief 获取流名称
             * @return 流名称字符串的常量引用
             */
            const string &StreamName() const;

            /**
             * @brief 设置流名称
             * @param stream 流名称字符串
             */
            void SetStreamName(const string &stream);

            /**
             * @brief 获取URL参数
             * @return 参数字符串的常量引用
             */
            const string &Param() const;

            /**
             * @brief 设置URL参数
             * @param param 参数字符串
             */
            void SetParam(const string &param);

            /**
             * @brief 获取应用信息指针
             * @return 应用信息指针的常量引用
             */
            const AppInfoPtr &GetAppInfo() const;

            /**
             * @brief 设置应用信息指针
             * @param info 应用信息指针
             */
            void SetAppInfo(const AppInfoPtr &info);

            /**
             * @brief 获取用户类型
             * @return 用户类型枚举值
             */
            virtual UserType GetUserType() const;

            /**
             * @brief 设置用户类型
             * @param t 用户类型枚举值
             */
            void SetUserType(UserType t);

            /**
             * @brief 获取用户协议
             * @return 用户协议枚举值
             */
            virtual UserProtocol GetUserProtocol() const;

            /**
             * @brief 设置用户协议
             * @param p 用户协议枚举值
             */
            void SetUserProtocol(UserProtocol p);

            /**
             * @brief 关闭用户连接
             */
            void Close();

            /**
             * @brief 获取连接指针
             * @return 连接指针
             */
            ConnectionPtr GetConnection();

            /**
             * @brief 获取用户连接已经存在的时间
             * @return 经过的时间（毫秒）
             */
            uint64_t ElapsedTime();

            /**
             * @brief 激活用户
             */
            void Active();

            /**
             * @brief 注销用户
             */
            void Deactive();

            /**
             * @brief 获取用户ID
             * @return 用户ID字符串的常量引用
             */
            const std::string &UserId() const
            {
                return user_id_;
            }

            /**
             * @brief 获取会话指针
             * @return 会话指针
             */
            SessionPtr GetSession() const
            {
                return session_;
            }

            /**
             * @brief 获取流指针
             * @return 流指针
             */
            StreamPtr GetStream() const
            {
                return stream_;
            }

            /**
             * @brief 虚析构函数
             */
            virtual ~User() = default;

          protected:
            ConnectionPtr connection_;                                   ///< 连接指针
            StreamPtr stream_;                                           ///< 流指针
            string domain_name_;                                         ///< 域名
            string app_name_;                                            ///< 应用名称
            string stream_name_;                                         ///< 流名称
            string param_;                                               ///< URL参数
            string user_id_;                                             ///< 用户ID
            AppInfoPtr app_info_;                                        ///< 应用信息指针
            int64_t start_timestamp_{0};                                 ///< 开始时间戳
            UserType type_{UserType::kUserTypeUnknowed};                 ///< 用户类型
            UserProtocol protocol_{UserProtocol::kUserProtocolUnknowed}; ///< 用户协议
            std::atomic_bool destroyed_{false};                          ///< 是否被销毁标志
            SessionPtr session_;                                         ///< 会话指针
        };
    } // namespace live
} // namespace tmms