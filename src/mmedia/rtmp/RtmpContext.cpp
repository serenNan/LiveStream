#include "RtmpContext.h"
#include "BytesReader.h"
#include "mmedia/base/MMediaLog.h"
using namespace tmms::mm;

RtmpContext::RtmpContext(const TcpConnectionPtr &conn, RtmpHandler *handler, bool client)
    : handshake_(conn, client), connection_(conn), rtmp_handler_(handler), is_client_(client)
{
}

int32_t RtmpContext::Parse(MsgBuffer &buff)
{
    int32_t ret = 0;

    // 检查当前状态是否为握手阶段
    if (state_ == kRtmpHandShake)
    {
        // 调用握手处理函数
        ret = handshake_.HandShake(buff);

        // 握手成功，状态切换到消息处理阶段
        if (ret == 0)
        {
            state_ = kRtmpMessage;

            // 如果缓冲区还有可读数据，继续解析
            if (buff.ReadableBytes() > 0)
            {
                // 递归调用，继续解析剩余数据
                return Parse(buff);
            }
        }
        else if (ret == -1)
        {
            // 握手失败，打印错误日志
            RTMP_ERROR << " rtmp handshake error.";
        }
        else if (ret == 2)
        {
            // 握手部分完成，切换到等待完成阶段
            state_ = kRtmpWatingDone;
        }
    }
    else if (state_ == kRtmpMessage)
    {
        ret = ParseMessage(buff);
    }

    // 返回结果，可能是 0, -1, 或 2，取决于握手结果
    return ret;
}

void RtmpContext::OnWriteComplete()
{
    // 如果当前状态是握手阶段，调用握手写入完成处理函数
    if (state_ == kRtmpHandShake)
    {
        handshake_.WriteComplete();
    }
    // 如果当前状态是等待完成阶段，切换到消息处理阶段
    else if (state_ == kRtmpWatingDone)
    {
        state_ = kRtmpMessage;
    }
    // 消息处理阶段写入完成后的操作，不做处理
    else if (state_ == kRtmpMessage)
    {
    }
}

void RtmpContext::StartHandShake()
{
    // 调用握手开始函数，启动握手流程
    handshake_.Start();
}

int32_t RtmpContext::ParseMessage(MsgBuffer &buff)
{
    // 消息格式（FMT）- 决定了消息头的格式类型（0-3）
    uint8_t fmt;
    // 消息通道ID（CSID）- 标识消息所属的通道
    // 消息长度（msg_len）- 表示消息体的字节数
    // 消息流ID（msg_sid）- 标识消息所属的流
    uint32_t csid, msg_len = 0, msg_sid = 0, timestamp = 0;
    // 消息类型（msg_type）- 标识消息的类型，如音频、视频、命令等
    uint8_t msg_type = 0;
    // 缓冲区内可读字节数 - 用于判断是否有足够的数据可以解析
    uint32_t total_bytes = buff.ReadableBytes();
    // 已解析的字节数 - 记录当前解析进度
    int32_t parsed = 0;

    while (total_bytes > 1) // 至少需要1个字节才能开始解析
    {
        // 获取当前缓冲区指针，用于读取数据
        const char *pos = buff.Peek();

        parsed = 0; // 重置已解析字节计数

        // 从第一个字节的高2位提取FMT值（0-3）
        fmt = (*pos >> 6) & 0x03;

        // 从第一个字节的低6位提取基本CSID值
        csid = *pos & 0x3F;
        parsed++; // 已解析1个字节

        // RTMP协议中CSID的特殊情况处理：
        if (csid == 0) // 如果CSID为0，表示实际CSID需要额外1个字节表示
        {
            // 检查是否有足够的数据
            if (total_bytes < 2)
            {
                return 1; // 数据不足，需要更多数据
            }

            // CSID = 64 + 第二个字节的值（范围：64-319）
            csid = 64;
            // 读取第二个字节作为CSID的增量值
            csid += *((uint8_t *)(pos + parsed));
            parsed++; // 已解析2个字节
        }
        else if (csid == 1) // 如果CSID为1，表示实际CSID需要额外2个字节表示
        {
            // 检查是否有足够的数据
            if (total_bytes < 3)
            {
                return 1; // 数据不足，需要更多数据
            }

            // CSID = 64 + 第二个字节 + 第三个字节*256（范围：64-65599）
            csid = 64;
            // 读取第二个字节（低8位）
            csid += *((uint8_t *)(pos + parsed));
            parsed++; // 已解析2个字节
            // 读取第三个字节（高8位）并计算实际值
            csid += *((uint8_t *)(pos + parsed)) * 256;
            parsed++; // 已解析3个字节
        }
        int size = total_bytes - parsed;

        if (size == 0 || (fmt == 0 && size < 11) || (fmt == 1 && size < 7) ||
            (fmt == 2 && size < 3))
        {
            // 数据不足，返回 1 表示需要更多数据
            return 1;
        }
        // 初始化消息长度
        msg_len = 0;
        // 初始化流 ID
        msg_sid = 0;
        // 初始化消息类型
        msg_type = 0;
        // 定义新的时间戳
        timestamp = 0;
        int32_t ts = 0;

        // 获取上一次的消息头部，使用 CSID 作为键
        RtmpMsgHeaderPtr &prev = in_message_headers_[csid];

        if (!prev)
        {
            // 如果之前没有消息头部，创建一个新的
            prev = std::make_shared<RtmpMsgHeader>();
        }

        if (fmt == kRtmpFmt0)
        {
            ts = BytesReader::ReadUint24T(pos + parsed);
            parsed += 3;
            in_deltas_[csid] = 0;
            timestamp = ts;
            msg_len = BytesReader::ReadUint24T(pos + parsed);
            parsed += 3;
            msg_type = BytesReader::ReadUint8T(pos + parsed);
            parsed += 1;
            memcpy(&msg_sid, pos + parsed, 4);
            parsed += 4;
        }
        else if (fmt == kRtmpFmt1)
        {
            ts = BytesReader::ReadUint24T(pos + parsed);
            parsed += 3;
            in_deltas_[csid] = ts;
            timestamp = ts + prev->timestamp;
            msg_len = BytesReader::ReadUint24T(pos + parsed);
            parsed += 3;
            msg_type = BytesReader::ReadUint8T(pos + parsed);
            parsed += 1;
            msg_sid = prev->msg_sid;
        }
        else if (fmt == kRtmpFmt2)
        {
            ts = BytesReader::ReadUint24T(pos + parsed);
            parsed += 3;
            in_deltas_[csid] = ts;
            timestamp = ts + prev->timestamp;
            msg_len = prev->msg_len;
            msg_type = prev->msg_type;
            msg_sid = prev->msg_sid;
        }
        else if (fmt == kRtmpFmt3)
        {
            timestamp = ts + prev->timestamp;
            msg_len = prev->msg_len;
            msg_type = prev->msg_type;
            msg_sid = prev->msg_sid;
        }

        bool ext = (ts == 0xFFFFFF);

        if (fmt == kRtmpFmt3) // FMT3 使用之前保存的扩展时间戳标志
        {
            ext = in_ext_[csid];
        }

        // 更新扩展时间戳标志
        in_ext_[csid] = ext;

        // 如果使用扩展时间戳
        if (ext)
        {
            // 检查是否有足够的字节用于读取扩展时间戳
            if (total_bytes - parsed < 4)
            {
                // 数据不足，返回 1 表示需要更多数据
                return 1;
            }

            timestamp = BytesReader::ReadUint32T(pos + parsed);
            parsed += 4;
            if (fmt != kRtmpFmt0)
            {
                timestamp += ts + prev->timestamp;
                in_deltas_[csid] = ts;
            }
        }
        // 获取当前 CSID 对应的数据包指针
        PacketPtr &packet = in_packets_[csid];

        // 如果尚未初始化数据包
        if (!packet)
        {
            // 创建一个新数据包，大小为消息长度
            packet = Packet::NewPacket(msg_len);
        }

        // 从数据包中获取消息头部的扩展信息
        RtmpMsgHeaderPtr header = packet->Ext<RtmpMsgHeader>();

        // 如果数据包中没有消息头部扩展信息
        if (!header)
        {
            // 创建新的消息头部
            header = std::make_shared<RtmpMsgHeader>();
            // 将消息头部设置为数据包的扩展信息
            packet->SetExt(header);
        }

        // 更新消息头部信息
        header->cs_id = csid;
        header->msg_len = msg_len;
        header->msg_sid = msg_sid;
        header->msg_type = msg_type;
        header->timestamp = timestamp;

        int bytes = std::min(packet->Space(), in_chunk_size_);
        if (total_bytes - parsed < bytes)
        {
            return 1;
        }

        const char *body = packet->Data() + packet->PacketSize();
        memcpy((void *)body, pos + parsed, bytes);
        packet->UpdatePacketSize(bytes);
        parsed += bytes;

        buff.Retrieve(parsed);
        total_bytes -= parsed;

        prev->cs_id = csid;
        prev->msg_len = msg_len;
        prev->msg_sid = msg_sid;
        prev->msg_type = msg_type;
        prev->timestamp = timestamp;

        if (packet->Space() == 0)
        {
            packet->SetPacketSize(msg_type);
            packet->SetTimeStamp(timestamp);
            MessageComplete(std::move(packet));
            packet.reset();
        }
    }
    return 1;
}

void RtmpContext::MessageComplete(PacketPtr &&data)
{
    RTMP_TRACE << "recv message type:" << data->PacketType() << " size:" << data->PacketSize()
               << std::endl;
}
