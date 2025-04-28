#pragma once
#include "RtmpHandShake.h"
#include "RtmpHandler.h"
#include "RtmpHeader.h"
#include "mmedia/base/Packet.h"
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

          private:
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

            int32_t in_chunk_size_{128};
        };

        using RtmpContextPtr = std::shared_ptr<RtmpContext>;
    } // namespace mm
} // namespace tmms