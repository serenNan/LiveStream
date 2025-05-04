#pragma once
#include "PlayerUser.h"

namespace tmms
{
    namespace live
    {
        /**
         * @brief RTMP播放用户类，继承自PlayerUser，负责处理RTMP协议的播放
         */
        class RtmpPlayerUser : public PlayerUser
        {
          public:
            /**
             * @brief 构造函数
             * @param ptr 连接指针
             * @param stream 流指针
             * @param s 会话指针
             */
            explicit RtmpPlayerUser(const ConnectionPtr &ptr, const StreamPtr &stream,
                                    const SessionPtr &s);

            /**
             * @brief 实现发布帧到客户端的功能
             * @return 发布是否成功
             */
            bool PostFrames() override;

            /**
             * @brief 获取用户类型
             * @return 返回RTMP播放器用户类型
             */
            UserType GetUserType() const override;

          private:
            /**
             * @brief 使用基类的SetUserType方法
             */
            using User::SetUserType;

            /**
             * @brief 推送单个帧到客户端
             * @param packet 要推送的数据包
             * @param is_header 是否为头信息帧
             * @return 推送是否成功
             */
            bool PushFrame(PacketPtr &packet, bool is_header);

            /**
             * @brief 推送多个帧到客户端
             * @param list 要推送的数据包列表
             * @return 推送是否成功
             */
            bool PushFrames(std::vector<PacketPtr> &list);
        };
    } // namespace live
} // namespace tmms