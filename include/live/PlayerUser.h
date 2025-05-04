#pragma once
#include "User.h"
#include "live/base/TimeCorrector.h"
#include "mmedia/base/Packet.h"
#include <vector>

namespace tmms
{
    namespace live
    {
        using namespace tmms::mm;

        /**
         * @brief 播放用户类，继承自User类，负责处理播放相关的功能
         */
        class PlayerUser : public User
        {
          public:
            /**
             * @brief 声明Stream类为友元类，允许Stream类访问PlayerUser的私有成员
             */
            friend class Stream;

            /**
             * @brief 构造函数
             * @param ptr 连接指针
             * @param stream 流指针
             * @param s 会话指针
             */
            explicit PlayerUser(const ConnectionPtr &ptr, const StreamPtr &stream,
                                const SessionPtr &s);

            /**
             * @brief 获取元数据
             * @return 元数据包的指针
             */
            PacketPtr Meta() const;

            /**
             * @brief 获取视频头信息
             * @return 视频头信息包的指针
             */
            PacketPtr VideoHeader() const;

            /**
             * @brief 获取音频头信息
             * @return 音频头信息包的指针
             */
            PacketPtr AudioHeader() const;

            /**
             * @brief 清除元数据
             */
            void ClearMeta();

            /**
             * @brief 清除音频头信息
             */
            void ClearAudioHeader();

            /**
             * @brief 清除视频头信息
             */
            void ClearVideoHeader();

            /**
             * @brief 发布帧到客户端
             * @return 发布是否成功
             */
            virtual bool PostFrames() = 0;

            /**
             * @brief 获取时间校正器
             * @return 时间校正器的引用
             */
            TimeCorrector &GetTimeCorrector();

          protected:
            PacketPtr video_header_;            ///< 视频头信息的指针
            PacketPtr audio_header_;            ///< 音频头信息的指针
            PacketPtr meta_;                    ///< 元数据的指针
            bool wait_meta_{true};              ///< 是否等待元数据
            bool wait_audio_{true};             ///< 是否等待音频
            bool wait_video_{true};             ///< 是否等待视频
            int32_t video_header_index_{0};     ///< 视频头索引
            int32_t audio_header_index_{0};     ///< 音频头索引
            int32_t meta_index_{0};             ///< 元数据索引
            TimeCorrector time_corrector_;      ///< 时间校正器对象
            bool wait_timeout_{false};          ///< 是否等待超时
            int32_t out_version_{-1};           ///< 输出版本
            int32_t out_frame_timestamp_{0};    ///< 输出帧时间戳
            std::vector<PacketPtr> out_frames_; ///< 输出帧的指针向量
            int32_t out_index_{-1};             ///< 输出索引
        };
    } // namespace live
} // namespace tmms