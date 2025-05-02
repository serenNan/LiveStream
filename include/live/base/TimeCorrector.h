#pragma once
#include "mmedia/base/Packet.h"
#include <cstdint>

namespace tmms
{
    namespace live
    {
        using namespace tmms::mm;

        /**
         * @brief 时间戳校正器类，负责音视频同步的时间戳校正
         *
         * 此类用于处理音视频流中的时间戳不同步问题，通过一系列算法
         * 确保音频和视频时间戳在一个合理的范围内同步，避免播放时
         * 出现音画不同步的情况。
         */
        class TimeCorrector
        {
            const int32_t kMaxVideoDeltaTime = 100;     ///< 视频时间戳最大允许误差(毫秒)，默认100ms
            const int32_t kMaxAudioDeltaTime = 100;     ///< 音频时间戳最大允许误差(毫秒)，默认100ms
            const int32_t kDefaultVideoDeltaTime = 40;  ///< 视频时间戳默认校正间隔(毫秒)，默认40ms
            const int32_t kDefaultAudioDeltaTime = 20;  ///< 音频时间戳默认校正间隔(毫秒)，默认20ms

          public:
            
            TimeCorrector() = default;

            /**
             * @brief 校正数据包时间戳的统一入口函数
             *
             * @param packet 需要校正时间戳的数据包
             * @return uint32_t 校正后的时间戳(毫秒)
             *
             * 根据数据包类型(音频/视频)自动选择合适的校正方法
             */
            uint32_t CorrectTimestamp(const PacketPtr &packet);

            /**
             * @brief 基于视频时间戳校正音频时间戳
             *
             * @param packet 音频数据包
             * @return uint32_t 校正后的音频时间戳(毫秒)
             *
             * 使用已校正的视频时间戳作为基准，校正音频时间戳，
             * 确保音频与最近的视频帧保持正确的时间关系
             */
            uint32_t CorrectAudioTimeStampByVideo(const PacketPtr &packet);

            /**
             * @brief 基于历史视频时间戳校正当前视频时间戳
             *
             * @param packet 视频数据包
             * @return uint32_t 校正后的视频时间戳(毫秒)
             *
             * 通过比较当前视频帧与前一视频帧的时间差，
             * 检测并修正异常的时间跳变，保证视频播放的流畅性
             */
            uint32_t CorrectVideoTimeStampByVideo(const PacketPtr &packet);

            /**
             * @brief 基于历史音频时间戳校正当前音频时间戳
             *
             * @param packet 音频数据包
             * @return uint32_t 校正后的音频时间戳(毫秒)
             *
             * 通过比较当前音频帧与前一音频帧的时间差，
             * 检测并修正异常的时间跳变，保证音频播放的连续性
             */
            uint32_t CorrectAudioTimeStampByAudio(const PacketPtr &packet);

            ~TimeCorrector() = default;

          private:
            int64_t video_original_timestamp_{-1}; ///< 原始视频时间戳，记录未经校正的最近一帧视频的时间戳，默认值-1表示未收到任何视频帧
            
            int64_t video_corrected_timestamp_{0}; ///< 校正后的视频时间戳，存储经过校正后的最近一帧视频的时间戳，默认值0
            
            int64_t audio_original_timestamp_{-1}; ///< 原始音频时间戳，记录未经校正的最近一帧音频的时间戳，默认值-1表示未收到任何音频帧
            
            int64_t audio_corrected_timestamp_{0}; ///< 校正后的音频时间戳，存储经过校正后的最近一帧音频的时间戳，默认值0
            
            int32_t audio_numbers_between_video_{0}; ///< 两个视频帧之间的音频帧计数，用于在音视频校正过程中计算音频帧的合理时间戳增量，默认值0
        };
    } // namespace live
} // namespace tmms