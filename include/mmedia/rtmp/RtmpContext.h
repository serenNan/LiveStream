#pragma once
#include "RtmpHandShake.h"
#include "RtmpHandler.h"
#include "RtmpHeader.h"
#include "base/Packet.h"
#include "amf/AMFObject.h"
#include "network/net/TcpConnection.h"
#include <cstdint>
#include <unordered_map>

namespace tmms
{
    namespace mm
    {
        using namespace tmms::network;

        // RTMP上下文状态枚举，表示当前RTMP连接所处的阶段
        enum RtmpContextState
        {
            kRtmpHandShake = 0,  // RTMP 握手阶段
            kRtmpWatingDone = 1, // RTMP 正在等待完成握手的阶段
            kRtmpMessage = 2     // RTMP 已经进入消息处理阶段
        };

        // RTMP事件类型枚举，描述RTMP协议中常见的事件类型
        enum RtmpEventType
        {
            kRtmpEventTypeStreamBegin =
                0, // 表示流开始事件。当客户端或服务器开始发送音视频流时，会触发此事件
            kRtmpEventTypeStreamEOF, // 表示流结束事件。当音视频流传输结束时，会触发此事件
            kRtmpEventTypeStreamDry, // 表示流干涸事件。意味着当前流没有更多数据可发送，可能是由于网络问题或其他原因导致的流中断
            kRtmpEventTypeSetBufferLength, // 设置缓冲区长度事件。用于调整客户端的缓冲区大小，避免因缓冲区溢出或不足而导致的流播放问题
            kRtmpEventTypeStreamsRecorded, // 流录制事件。表示流已经被记录下来，通常用于点播或录像的场景
            kRtmpEventTypePingRequest, // Ping请求事件。用于检测连接的延迟或确认连接的有效性，类似于网络中的ICMP
                                       // Ping
            kRtmpEventTypePingResponse // Ping响应事件。对Ping请求的响应，确认连接状态
        };

        using CommandFunc = std::function<void(AMFObject &obj)>;
        
        // RTMP上下文类，负责管理一次RTMP连接的状态、握手、消息解析等
        class RtmpContext
        {
          public:
            /**
             * @brief 构造函数，初始化RTMP上下文
             * @param conn TCP连接指针
             * @param handler RTMP处理器指针
             * @param client 是否为客户端模式
             */
            RtmpContext(const TcpConnectionPtr &conn, RtmpHandler *handler, bool client = false);
            ~RtmpContext() = default;

            /**
             * @brief 解析收到的消息缓冲区，处理RTMP协议数据
             * @param buff 消息缓冲区
             * @return 解析结果，0表示成功，其他为错误码
             */
            int32_t Parse(MsgBuffer &buff);

            /**
             * @brief 写操作完成后的回调函数
             */
            void OnWriteComplete();

            /**
             * @brief 启动握手流程
             */
            void StartHandShake();

            /**
             * @brief 解析RTMP消息
             *
             * 当握手完成后，解析RTMP消息体，包括消息头和消息体
             *
             * @param buff 消息缓冲区
             * @return int32_t 解析结果，0表示成功，其他为错误码
             */
            int32_t ParseMessage(MsgBuffer &buff);

            /**
             * @brief 消息完成处理
             *
             * 当一个完整的RTMP消息接收完毕后调用此函数进行处理
             *
             * @param data 完整的消息数据包
             */
            void MessageComplete(PacketPtr &&data);

            /**
             * @brief 构建RTMP数据块
             * @param packet 数据包指针
             * @param timestamp 时间戳，默认为0
             * @param fmt0 是否使用格式0，默认为false
             * @return 构建是否成功
             */
            bool BuildChunk(const PacketPtr &packet, uint32_t timestamp = 0, bool fmt0 = false);

            /**
             * @brief 发送数据
             * 
             * 将缓冲区中的数据发送出去
             */
            void Send();

            /**
             * @brief 检查连接是否就绪
             * @return 连接是否就绪
             */
            bool Ready() const;

            /**
             * @brief 播放指定URL的媒体流
             * @param url 要播放的媒体流URL
             */
            void Play(const std::string &url);

            /**
             * @brief 发布媒体流到指定URL
             * @param url 要发布到的URL
             */
            void Publish(const std::string &url);

          private:
            /**
             * @brief 构建RTMP数据块（移动语义版本）
             * @param packet 数据包指针（移动）
             * @param timestamp 时间戳，默认为0
             * @param fmt0 是否使用格式0，默认为false
             * @return 构建是否成功
             */
            bool BuildChunk(PacketPtr &&packet, uint32_t timestamp = 0, bool fmt0 = false);

            /**
             * @brief 检查并发送数据
             * 
             * 检查是否有数据需要发送，如果有则发送
             */
            void CheckAndSend();

            /**
             * @brief 将数据包推入输出队列
             * @param packet 要发送的数据包（移动）
             */
            void PushOutQueue(PacketPtr &&packet);

            /**
             * @brief 处理块大小消息
             * 
             * 处理接收到的设置RTMP块大小的控制消息
             * @param packet 包含块大小信息的数据包
             */
            void HandleChunkSize(PacketPtr &packet);

            /**
             * @brief 处理确认窗口大小消息
             * 
             * 处理接收到的确认窗口大小的控制消息，用于流量控制
             * @param packet 包含窗口大小信息的数据包
             */
            void HandleAckWindowSize(PacketPtr &packet);

            /**
             * @brief 处理用户控制消息
             * 
             * 处理接收到的用户控制消息，如ping、buffer等
             * @param packet 包含用户控制消息的数据包
             */
            void HandleUserMessage(PacketPtr &packet);

            /**
             * @brief 处理AMF命令消息
             * 
             * 处理接收到的AMF命令消息，如connect、createStream等
             * @param data 包含AMF命令的数据包
             * @param amf3 是否为AMF3格式，默认为false（AMF0格式）
             */
            void HandleAmfCommand(PacketPtr &data, bool amf3 = false);

            /**
             * @brief 发送设置块大小消息
             * 
             * 向对端发送设置RTMP块大小的控制消息
             */
            void SendSetChunkSize();

            /**
             * @brief 发送确认窗口大小消息
             * 
             * 向对端发送确认窗口大小的控制消息，用于流量控制
             */
            void SendAckWindowSize();

            /**
             * @brief 发送设置对端带宽消息
             * 
             * 向对端发送带宽限制信息，用于流量控制
             */
            void SendSetPeerBandwidth();

            /**
             * @brief 发送字节接收确认消息
             * 
             * 当接收的字节数达到确认窗口大小时，发送确认消息
             */
            void SendBytesRecv();

            // /**
            //  * @brief 设置数据包类型
            //  * 
            //  * 根据RTMP消息类型设置数据包的类型标识，用于后续处理
            //  * @param packet 需要设置类型的数据包
            //  */
            // void SetPacketType(PacketPtr &packet);

            /**
             * @brief 发送用户控制消息
             * 
             * @param nType 控制消息类型
             * @param value1 第一个参数值
             * @param value2 第二个参数值，用于某些特定类型的控制消息
             */
            void SendUserCtrlMessage(short nType, uint32_t value1, uint32_t value2);

            RtmpHandShake handshake_;            ///< RTMP握手处理对象
            int32_t state_{kRtmpHandShake};      ///< 当前上下文状态，参见RtmpContextState
            TcpConnectionPtr connection_;        ///< TCP连接指针
            RtmpHandler *rtmp_handler_{nullptr}; ///< RTMP处理器指针
            bool is_client_{false};              ///< 是否为客户端模式

            // 存储接收到的消息头部
            std::unordered_map<uint32_t, RtmpMsgHeaderPtr> in_message_headers_;
            // 存储接收到的数据包
            std::unordered_map<uint32_t, PacketPtr> in_packets_;
            // 存储时间戳增量信息
            std::unordered_map<uint32_t, uint32_t> in_deltas_;
            // 标记是否使用扩展时间戳
            std::unordered_map<uint32_t, bool> in_ext_;

            int32_t in_chunk_size_{128}; ///< 输入数据块大小，默认为128字节

            char out_buffer_[4096]; ///< 输出缓冲区

            char *out_current_{nullptr}; ///< 当前输出缓冲区位置指针

            std::unordered_map<uint32_t, uint32_t> out_deltas_; ///< 输出时间戳增量信息

            std::unordered_map<uint32_t, RtmpMsgHeaderPtr>
                out_message_headers_; ///< 存储发送消息的头部信息

            int32_t out_chunk_size_{4096}; ///< 输出数据块大小，默认为4096字节

            std::list<PacketPtr> out_waiting_queue_; ///< 等待发送的数据包队列

            std::list<BufferNodePtr> sending_bufs_; ///< 正在发送的缓冲区列表

            std::list<PacketPtr> out_sending_packets_; ///< 正在发送的数据包列表

            bool sending_{false}; ///< 标记当前是否正在发送数据

            int32_t ack_size_{2500000}; ///< 确认窗口大小，默认为2.5MB，当接收的数据量达到此值时需要发送确认包

            int32_t in_bytes_{0}; ///< 已接收的字节数，用于跟踪何时需要发送确认包

            int32_t last_left_{0}; ///< 上次确认后剩余的字节数，用于计算下一次确认时机

            std::unordered_map<std::string, CommandFunc> commands_;
        };

        using RtmpContextPtr = std::shared_ptr<RtmpContext>;
    } // namespace mm
} // namespace tmms