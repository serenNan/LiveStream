#pragma once
#include "PlayerUser.h"
#include "User.h"
#include "base/AppInfo.h"
#include <atomic>
#include <mutex>
#include <string>
#include <unordered_set>

namespace tmms
{
    namespace live
    {
        /**
         * @brief PlayerUserPtr是PlayerUser的共享指针类型别名
         */
        using PlayerUserPtr = std::shared_ptr<PlayerUser>;

        /**
         * @brief UserPtr是User的共享指针类型别名
         */
        using UserPtr = std::shared_ptr<User>;

        /**
         * @brief 会话类，管理直播流的发布者和播放者
         *
         * Session类继承自std::enable_shared_from_this，允许对象安全地创建指向自身的shared_ptr
         */
        class Session : public std::enable_shared_from_this<Session>
        {
          public:
            /**
             * @brief 构造函数
             * @param session_name 会话名称
             */
            explicit Session(const std::string &session_name);

            /**
             * @brief 获取会话准备就绪的时间
             * @return 准备时间的32位整数值
             */
            int32_t ReadyTime() const;

            /**
             * @brief 获取自会话开始以来经过的时间
             * @return 经过时间的64位整数值(毫秒)
             */
            int64_t SinceStart() const;

            /**
             * @brief 检查会话是否已超时
             * @return 如果超时返回true，否则返回false
             */
            bool IsTimeout();

            /**
             * @brief 创建发布者用户
             * @param conn 连接指针
             * @param session_name 会话名称
             * @param param 参数字符串
             * @param type 用户类型
             * @return 创建的用户指针
             */
            UserPtr CreatePublishUser(const ConnectionPtr &conn, const std::string &session_name,
                                      const std::string &param, UserType type);

            /**
             * @brief 创建播放用户
             * @param conn 连接指针
             * @param session_name 会话名称
             * @param param 参数字符串
             * @param type 用户类型
             * @return 创建的用户指针
             */
            UserPtr CreatePlayerUser(const ConnectionPtr &conn, const std::string &session_name,
                                     const std::string &param, UserType type);

            /**
             * @brief 关闭指定的用户连接
             * @param user 要关闭的用户指针
             */
            void CloseUser(const UserPtr &user);

            /**
             * @brief 激活所有播放用户
             */
            void ActiveAllPlayers();

            /**
             * @brief 添加播放用户到会话
             * @param user 要添加的播放用户指针
             */
            void AddPlayer(const PlayerUserPtr &user);

            /**
             * @brief 设置会话的发布者
             * @param user 发布者用户指针
             */
            void SetPublisher(UserPtr &user);

            /**
             * @brief 获取会话管理的流对象
             * @return 流对象指针
             */
            StreamPtr GetStream();

            /**
             * @brief 获取会话名称
             * @return 会话名称的常量引用
             */
            const string &SessionName() const;

            /**
             * @brief 设置应用程序信息
             * @param ptr 应用程序信息指针
             */
            void SetAppInfo(AppInfoPtr &ptr);

            /**
             * @brief 获取应用程序信息
             * @return 应用程序信息指针的引用
             */
            AppInfoPtr &GetAppInfo();

            /**
             * @brief 判断会话是否有活跃的发布者
             * @return 如果正在发布返回true，否则返回false
             */
            bool IsPublishing() const;

            /**
             * @brief 清除会话的所有状态，包括发布者和播放者
             */
            void Clear();

          private:
            /**
             * @brief 在不加锁的情况下关闭用户
             * @param user 要关闭的用户指针
             */
            void CloseUserNoLock(const UserPtr &user);

            std::string session_name_;                  ///< 会话名称
            std::unordered_set<PlayerUserPtr> players_; ///< 播放用户集合
            AppInfoPtr app_info_;                       ///< 应用程序信息指针
            StreamPtr stream_;                          ///< 流对象指针
            UserPtr publisher_;                         ///< 发布者用户指针
            std::mutex lock_;                           ///< 互斥锁，用于线程同步
            std::atomic<int64_t> player_live_time_;     ///< 玩家活动时间，原子类型
        };
    } // namespace live
} // namespace tmms