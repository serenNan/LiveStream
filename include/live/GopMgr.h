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
         * @brief GOP项信息结构体，用于存储GOP的索引和时间戳
         */
        struct GopItemInfo
        {
            int32_t index;     ///< GOP项的索引
            int64_t timestamp; ///< GOP项的时间戳

            /**
             * @brief 构造函数
             * @param i GOP索引
             * @param t GOP时间戳
             */
            GopItemInfo(int32_t i, int64_t t) : index(i), timestamp(t)
            {
            }
        };

        /**
         * @brief GOP管理器类，用于管理视频GOP（图像组）
         */
        class GopMgr
        {
          public:
            /**
             * @brief 默认构造函数
             */
            GopMgr() = default;

            /**
             * @brief 添加一帧到GOP管理器
             * @param packet 媒体数据包
             */
            void AddFrame(const PacketPtr &packet);

            /**
             * @brief 获取最大GOP长度
             * @return 最大GOP长度
             */
            int32_t MaxGopLength() const;

            /**
             * @brief 获取当前GOP数量
             * @return 当前GOP数量
             */
            size_t GopSize() const;

            /**
             * @brief 根据延迟获取GOP索引
             * @param content_latency 内容延迟
             * @param latency 实际延迟（输出参数）
             * @return GOP索引
             */
            int GetGopByLatency(int content_latency, int &latency) const;

            /**
             * @brief 清除过期的GOP
             * @param min_idx 最小索引，低于此索引的GOP将被清除
             */
            void ClearExpriedGop(int min_idx);

            /**
             * @brief 打印所有GOP信息
             */
            void PrintAllGop();

            /**
             * @brief 获取最新时间戳
             * @return 最新时间戳
             */
            int64_t LastestTimeStamp() const
            {
                return lastest_timestamp_;
            }

            /**
             * @brief 析构函数
             */
            ~GopMgr(){};

          private:
            std::vector<GopItemInfo> gops_; ///< 存储GOP项的向量
            int32_t gop_length_{0};         ///< 当前GOP长度
            int32_t max_gop_length_{0};     ///< 最大GOP长度
            int32_t gop_numbers_{0};        ///< GOP项的数量
            int32_t total_gop_length_{0};   ///< 总GOP长度
            int64_t lastest_timestamp_{0};  ///< 最新时间戳
        };

    } // namespace live
} // namespace tmms