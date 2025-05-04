#pragma once
#include "PlayerUser.h"
#include "User.h"
#include "live/CodecHeader.h"
#include "live/GopMgr.h"
#include "live/base/TimeCorrector.h"
#include "mmedia/base/Packet.h"
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace tmms
{
    namespace live
    {
        using namespace tmms::mm;

        /**
         * @brief UserPtr是User的共享指针类型别名
         */
        using UserPtr = std::shared_ptr<User>;

        /**
         * @brief PlayerUserPtr是PlayerUser的共享指针类型别名
         */
        using PlayerUserPtr = std::shared_ptr<PlayerUser>;

        /**
         * @brief 前向声明Session类
         */
        class Session;

        /**
         * @brief 流类，负责管理和分发媒体流数据
         */
        class Stream
        {
          public:
            /**
             * @brief 构造函数
             * @param s Session引用
             * @param session_name 会话名称
             */
            Stream(Session &s, const std::string &session_name);

            /**
             * @brief 获取流准备就绪的时间
             * @return 准备时间的时间戳
             */
            int64_t ReadyTime() const;

            /**
             * @brief 获取自流开始以来经过的时间
             * @return 经过的时间（毫秒）
             */
            int64_t SinceStart() const;

            /**
             * @brief 检查流是否超时
             * @return 如果超时返回true，否则返回false
             */
            bool Timeout();

            /**
             * @brief 获取最新数据到达的时间
             * @return 数据时间的时间戳
             */
            int64_t DataTime() const;

            /**
             * @brief 获取会话名称
             * @return 会话名称的常量引用
             */
            const std::string &SessionName() const;

            /**
             * @brief 获取流当前版本号
             * @return 流版本号
             */
            int32_t StreamVersion() const;

            /**
             * @brief 检查流是否含有媒体数据
             * @return 如果有媒体数据返回true，否则返回false
             */
            bool HasMedia() const;

            /**
             * @brief 检查流是否准备就绪可以播放
             * @return 如果准备就绪返回true，否则返回false
             */
            bool Ready() const;

            /**
             * @brief 添加数据包到流中
             * @param packet 移动的数据包指针
             */
            void AddPacket(PacketPtr &&packet);

            /**
             * @brief 为特定用户获取帧数据
             * @param user 播放用户指针
             */
            void GetFrames(const PlayerUserPtr &user);

          private:
            /**
             * @brief 为特定用户定位GOP（图像组）
             * @param user 播放用户指针
             * @return 定位是否成功
             */
            bool LocateGop(const PlayerUserPtr &user);

            /**
             * @brief 为特定用户跳过帧
             * @param user 播放用户指针
             */
            void SkipFrame(const PlayerUserPtr &user);

            /**
             * @brief 为特定用户获取下一帧
             * @param user 播放用户指针
             */
            void GetNextFrame(const PlayerUserPtr &user);

            /**
             * @brief 设置流的准备状态
             * @param ready 准备状态
             */
            void SetReady(bool ready);

            int64_t data_coming_time_{0};             ///< 数据到达时间
            int64_t start_timestamp_{0};              ///< 流开始时间戳
            int64_t ready_time_{0};                   ///< 准备就绪时间
            std::atomic<int64_t> stream_time_{0};     ///< 流当前时间
            Session &session_;                        ///< Session引用
            std::string session_name_;                ///< 会话名称
            std::atomic<int64_t> frame_index_{-1};    ///< 当前帧索引
            uint32_t packet_buffer_size_{1000};       ///< 数据包缓冲区大小
            std::vector<PacketPtr> packet_buffer_;    ///< 数据包缓冲区
            bool has_audio_{false};                   ///< 是否有音频
            bool has_video_{false};                   ///< 是否有视频
            bool has_meta_{false};                    ///< 是否有元数据
            bool ready_{false};                       ///< 流是否准备就绪
            std::atomic<int32_t> stream_version_{-1}; ///< 流版本号
            GopMgr gop_mgr_;                          ///< GOP管理器
            CodecHeader codec_headers_;               ///< 编解码头信息
            TimeCorrector time_corrector_;            ///< 时间校正器
            std::mutex lock_;                         ///< 互斥锁，用于线程同步
        };
    } // namespace live
} // namespace tmms