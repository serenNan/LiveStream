#pragma once
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>

namespace tmms
{
    namespace mm
    {
        /**
         * @brief 媒体包类型和帧类型的枚举定义
         *
         * 使用位掩码方式定义，允许组合多个类型：
         * - 基本类型（1-8）：视频、音频、元数据
         * - 帧类型（16-32）：关键帧、IDR帧
         * - 特殊类型（255）：未知类型
         */
        enum PacketType
        {
            kPacketTypeVideo = 1,     ///< 视频包类型（0x01）
            kPacketTypeAudio = 2,     ///< 音频包类型（0x02）
            kPacketTypeMeta = 4,      ///< 元数据包类型（0x04）
            kPacketTypeMeta3 = 8,     ///< 扩展元数据包类型（0x08）
            kFrameTypeKeyFrame = 16,  ///< 视频关键帧标记（0x10）
            kFrameTypeIDR = 32,       ///< IDR帧标记（0x20），用于H.264/H.265编码
            kPacketTypeUnknowed = 255 ///< 未知包类型（0xFF）
        };

        class Packet;

        /**
         * @brief Packet 类的智能指针类型别名
         * 使用 shared_ptr 进行自动内存管理
         */
        using PacketPtr = std::shared_ptr<Packet>;

#pragma pack(push)
#pragma pack(1) // 1字节对齐，确保内存布局紧凑

        /**
         * @brief 媒体数据包类，用于封装各种类型的媒体数据
         *
         * 该类实现了一个通用的媒体数据包容器，具有以下特性：
         * - 支持视频、音频、元数据等多种媒体类型
         * - 使用内存紧凑布局，适合网络传输
         * - 提供类型安全的数据访问接口
         * - 支持扩展数据附加
         * - 包含时间戳和索引信息
         */
        class Packet
        {
          public:
            /**
             * @brief 构造一个指定容量的数据包
             * @param size 数据包的容量（字节）
             * @note 实际可用空间为 size - sizeof(Packet)
             */
            Packet(int32_t size) : capacity_(size)
            {
            }

            /**
             * @brief 创建一个新的数据包
             * @param size 数据包的容量（字节）
             * @return PacketPtr 指向新创建数据包的智能指针
             * @note 推荐使用此静态方法创建数据包实例
             */
            static PacketPtr NewPacket(int32_t size);

            /**
             * @brief 检查是否为视频包
             * @return bool true 表示是视频包
             * @note 使用位运算检查类型标志
             */
            bool IsVideo() const
            {
                return (type_ & kPacketTypeVideo) == kPacketTypeVideo;
            }

            /**
             * @brief 检查是否为视频关键帧
             * @return bool true 表示是视频关键帧
             * @note 同时检查视频类型和关键帧标志
             */
            bool IsKeyFrame() const
            {
                return ((type_ & kPacketTypeVideo) == kPacketTypeVideo) &&
                       ((type_ & kFrameTypeKeyFrame) == kFrameTypeKeyFrame);
            }

            /**
             * @brief 检查是否为音频包
             * @return bool true 表示是音频包
             */
            bool IsAudio() const
            {
                return type_ == kPacketTypeAudio;
            }

            /**
             * @brief 检查是否为元数据包
             * @return bool true 表示是元数据包
             */
            bool IsMeta() const
            {
                return type_ == kPacketTypeMeta;
            }

            /**
             * @brief 检查是否为扩展元数据包
             * @return bool true 表示是扩展元数据包
             */
            bool IsMeta3() const
            {
                return type_ == kPacketTypeMeta3;
            }

            /**
             * @brief 获取数据包中实际数据的大小
             * @return int32_t 数据大小（字节）
             */
            inline int32_t PacketSize() const
            {
                return size_;
            }

            /**
             * @brief 获取数据包中剩余的可用空间
             * @return int 剩余空间大小（字节）
             */
            inline int Space() const
            {
                return capacity_ - size_;
            }

            /**
             * @brief 设置数据包中实际数据的大小
             * @param len 数据大小（字节）
             */
            inline void SetPacketSize(size_t len)
            {
                size_ = len;
            }

            /**
             * @brief 增加数据包中实际数据的大小
             * @param len 要增加的大小（字节）
             */
            inline void UpdatePacketSize(size_t len)
            {
                size_ += len;
            }

            /**
             * @brief 设置数据包的索引值
             * @param index 索引值
             * @note 索引值可用于标识数据包的序列顺序
             */
            void SetIndex(int32_t index)
            {
                index_ = index;
            }

            /**
             * @brief 获取数据包的索引值
             * @return int32_t 索引值
             */
            int32_t Index() const
            {
                return index_;
            }

            /**
             * @brief 设置数据包的类型
             * @param type 包类型（参见 PacketType 枚举）
             */
            void SetPacketType(int32_t type)
            {
                type_ = type;
            }

            /**
             * @brief 获取数据包的类型
             * @return int32_t 包类型
             */
            int32_t PacketType() const
            {
                return type_;
            }

            /**
             * @brief 设置数据包的时间戳
             * @param timestamp 时间戳值（微秒）
             */
            void SetTimeStamp(uint64_t timestamp)
            {
                timestamp_ = timestamp;
            }

            /**
             * @brief 获取数据包的时间戳
             * @return uint64_t 时间戳值（微秒）
             */
            uint64_t TimeStamp() const
            {
                return timestamp_;
            }

            /**
             * @brief 获取数据包的数据区指针
             * @return char* 指向数据区的指针
             * @note 数据区紧跟在 Packet 结构体之后
             */
            inline char *Data()
            {
                return (char *)this + sizeof(Packet);
            }

            /**
             * @brief 获取扩展数据
             * @tparam T 扩展数据的类型
             * @return std::shared_ptr<T> 指向扩展数据的智能指针
             * @note 使用 RTTI 进行类型安全的转换
             */
            template <typename T> inline std::shared_ptr<T> Ext() const
            {
                return std::static_pointer_cast<T>(ext_);
            }

            /**
             * @brief 设置扩展数据
             * @param ext 指向扩展数据的智能指针
             */
            inline void SetExt(const std::shared_ptr<void> &ext)
            {
                ext_ = ext;
            }

            /**
             * @brief 析构函数
             */
            ~Packet()
            {
            }

          private:
            int32_t type_{kPacketTypeUnknowed}; ///< 包类型，默认为未知类型
            uint32_t size_{0};                  ///< 实际数据大小
            int32_t index_{-1};                 ///< 包索引，默认为-1
            uint64_t timestamp_{0};             ///< 时间戳（微秒）
            uint32_t capacity_{0};              ///< 包的总容量
            std::shared_ptr<void> ext_;         ///< 扩展数据指针

            // 注意：实际数据紧跟在此结构体之后
        };

#pragma pack(pop) // 恢复默认内存对齐

    } // namespace mm
} // namespace tmms