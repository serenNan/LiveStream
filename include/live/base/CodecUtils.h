#pragma once

#include "mmedia/base/Packet.h"

namespace tmms
{
    namespace live
    {
        using namespace tmms::mm;

        class CodecUtils
        {
        public:
            /**
             * @brief 检查包是否是编解码头
             * @param packet 待检查的数据包
             * @return 如果是编解码头返回true，否则返回false
             */
            static bool IsCodecHeader(const PacketPtr &packet);

            /**
             * @brief 检查包是否是关键帧
             * @param packet 待检查的数据包
             * @return 如果是关键帧返回true，否则返回false
             */
            static bool IsKeyFrame(const PacketPtr &packet);
        };
    }
}