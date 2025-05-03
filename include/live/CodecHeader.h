#pragma once
#include "mmedia/base/Packet.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace tmms
{
    namespace live
    {
        using namespace tmms::mm;

        /**
         * @brief CodecHeader类，处理音视频编解码相关的头信息
         *
         * 负责管理和处理流媒体传输中的编解码头信息，包括元数据、
         * 音频头信息和视频头信息的存储、解析和获取
         */
        class CodecHeader
        {
          public:
            CodecHeader();

            /**
             * @brief 获取第idx个元数据包
             *
             * @param idx 元数据包的索引
             * @return PacketPtr 指向元数据包的智能指针
             */
            PacketPtr Meta(int idx);

            /**
             * @brief 获取第idx个视频头信息包
             *
             * @param idx 视频头信息包的索引
             * @return PacketPtr 指向视频头信息包的智能指针
             */
            PacketPtr VideoHeader(int idx);

            /**
             * @brief 获取第idx个音频头信息包
             *
             * @param idx 音频头信息包的索引
             * @return PacketPtr 指向音频头信息包的智能指针
             */
            PacketPtr AudioHeader(int idx);

            /**
             * @brief 保存元数据包
             *
             * @param packet 要保存的元数据包
             *
             * 将元数据包存储到内部容器中，用于后续处理
             */
            void SaveMeta(const PacketPtr &packet);

            /**
             * @brief 解析元数据包
             *
             * @param packet 要解析的元数据包
             *
             * 从数据包中提取元数据信息并更新内部状态
             */
            void ParseMeta(const PacketPtr &packet);

            /**
             * @brief 保存音频头信息包
             *
             * @param packet 要保存的音频头信息包
             *
             * 将音频编解码头信息包存储到内部容器中
             */
            void SaveAudioHeader(const PacketPtr &packet);

            /**
             * @brief 保存视频头信息包
             *
             * @param packet 要保存的视频头信息包
             *
             * 将视频编解码头信息包存储到内部容器中
             */
            void SaveVideoHeader(const PacketPtr &packet);

            /**
             * @brief 解析编解码头信息
             *
             * @param packet 要解析的编解码头信息包
             * @return bool 解析是否成功，成功返回true，失败返回false
             *
             * 根据包类型自动识别并解析不同类型的编解码头信息
             */
            bool ParseCodecHeader(const PacketPtr &packet);

            
            ~CodecHeader();

          private:
            PacketPtr video_header_; ///< 存储当前视频头信息的包
            PacketPtr audio_header_; ///< 存储当前音频头信息的包
            PacketPtr meta_;         ///< 存储当前元数据包
            int meta_version_{0};    ///< 元数据版本号，用于跟踪元数据更新
            int audio_version_{0}; ///< 音频头信息版本号，用于跟踪音频编解码信息更新
            int video_version_{0}; ///< 视频头信息版本号，用于跟踪视频编解码信息更新
            std::vector<PacketPtr> video_header_packets_; ///< 存储多个视频头信息包的历史记录
            std::vector<PacketPtr> audio_header_packets_; ///< 存储多个音频头信息包的历史记录
            std::vector<PacketPtr> meta_packets_; ///< 存储多个元数据包的历史记录
            int64_t start_timestamp_{0}; ///< 编解码开始的时间戳，记录流的起始时间点
        };
    } // namespace live
} // namespace tmms