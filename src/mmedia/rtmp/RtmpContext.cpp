#include "RtmpContext.h"
#include "base/BytesReader.h"
#include "base/BytesWriter.h"
#include "base/MMediaLog.h"
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
        // 如果当前状态是消息处理阶段，调用消息解析函数
        auto r = ParseMessage(buff);

        // 获取缓冲区可读字节数
        last_left_ = buff.ReadableBytes();

        // 如果当前状态是消息处理阶段，调用消息解析函数
        // return ParseMessage(buff);

        // 如果缓冲区还有可读数据，继续解析
        return r;
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

    in_bytes_ += (buff.ReadableBytes() - last_left_);

    SendBytesRecv();

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
    // 当一个完整的消息包接收完毕时，打印消息类型和消息长度
    // RTMP_TRACE << " recv message type : " << data->PacketType() << " len : " <<
    // data->PacketSize() << std::endl;

    // 获取数据包的消息类型
    // 获取数据包的类型
    auto type = data->PacketType();

    switch (type)
    {
    // 处理 RTMP 消息类型为 Chunk Size（分块大小）的数据包
    case kRtmpMsgTypeChunkSize: {
        // 调用 HandleChunkSize 函数处理分块大小消息
        HandleChunkSize(data);
        break;
    }

    // 处理 RTMP 消息类型为 Bytes Read（字节读取）的数据包
    case kRtmpMsgTypeBytesRead: {
        // 输出日志信息，表示接收到字节读取消息
        RTMP_TRACE << " message bytes read recv.";
        break;
    }

    // 处理 RTMP 消息类型为 User Control（用户控制）的数据包
    case kRtmpMsgTypeUserControl: {
        // 调用 HandleUserMessage 函数处理用户控制消息
        HandleUserMessage(data);
        break;
    }

    // 处理 RTMP 消息类型为 Window ACK Size（窗口确认大小）的数据包
    case kRtmpMsgTypeWindowACKSize: {
        // 调用 HandleAckWindowSize 函数处理窗口确认大小消息
        HandleAckWindowSize(data);
        break;
    }

    // // 处理 AMF3 消息类型
    // case kRtmpMsgTypeAMF3Message: {
    //     // 调用 HandleAmfCommand 函数来处理 AMF3 类型的命令消息
    //     HandleAmfCommand(data, true);
    //     break;
    // }

    // // 处理 AMF 消息类型
    // case kRtmpMsgTypeAMFMessage: {
    //     // 调用 HandleAmfCommand 函数来处理 AMF 类型的命令消息
    //     HandleAmfCommand(data);
    //     break;
    // }

    // // 处理 AMF 元数据
    // case kRtmpMsgTypeAMFMeta:

    // // 处理 AMF3 元数据
    // case kRtmpMsgTypeAMF3Meta:

    // // 处理音频数据
    // case kRtmpMsgTypeAudio:

    // // 处理视频数据
    // case kRtmpMsgTypeVideo: {
    //     // 设置包的类型
    //     SetPacketType(data);

    //     // 如果 rtmp_handler_ 对象存在
    //     if (rtmp_handler_)
    //     {
    //         // 调用 rtmp_handler_ 的 OnRecv 方法，处理接收到的数据
    //         rtmp_handler_->OnRecv(connection_, std::move(data));
    //     }
    //     break;
    // }

    // 处理不支持的消息类型
    default:
        // 输出错误日志，表示收到的消息类型不被支持
        RTMP_ERROR << " not surpport message type : " << type;
        break;
    }
}

bool RtmpContext::BuildChunk(const PacketPtr &packet, uint32_t timestamp, bool fmt0)
{
    // 获取数据包的消息头
    RtmpMsgHeaderPtr h = packet->Ext<RtmpMsgHeader>();

    // 如果消息头存在，开始构建块
    if (h)
    {
        out_sending_packets_.emplace_back(packet);

        // 获取该 CSID 上次发送的消息头
        RtmpMsgHeaderPtr &prev = out_message_headers_[h->cs_id];
        // 判断是否可以使用时间戳增量，减少数据冗余
        bool use_delta =
            !fmt0 && !prev && timestamp >= prev->timestamp && h->msg_sid == prev->msg_sid;

        // 如果没有上次的消息头，则创建一个新的
        if (!prev)
        {
            prev = std::make_shared<RtmpMsgHeader>();
        }

        // 默认使用格式0
        int fmt = kRtmpFmt0;

        // 如果可以使用增量更新
        if (use_delta)
        {
            // 使用格式1
            fmt = kRtmpFmt1;
            // 计算时间戳的增量
            timestamp -= prev->timestamp;

            // 如果消息类型和长度相同，使用格式2
            if (h->msg_type == prev->msg_type && h->msg_len == prev->msg_len)
            {
                fmt = kRtmpFmt2;

                // 如果增量相同，使用格式3
                if (timestamp == out_deltas_[h->cs_id])
                {
                    fmt = kRtmpFmt3;
                }
            }
        }

        // 设置输出缓冲区当前指针
        char *p = out_current_;

        // 构建基本头部，根据 CSID 来决定如何编码
        if (h->cs_id < 64)
        {
            // 小于64，直接编码
            *p++ = (char)((fmt << 6) | h->cs_id);
        }
        else if (h->cs_id < (64 + 256))
        {
            // 使用1字节编码CSID
            *p++ = (char)((fmt << 6) | 0);
            *p++ = (char)(h->cs_id - 64);
        }
        else
        {
            // 使用2字节编码CSID
            *p++ = (char)((fmt << 6) | 1);
            uint16_t cs = h->cs_id - 64;
            memcpy(p, &cs, sizeof(uint16_t));
            p += sizeof(uint16_t);
        }

        // 时间戳处理，如果超过最大值，则使用最大值
        auto ts = timestamp;

        if (timestamp >= 0xFFFFFF)
        {
            ts = 0xFFFFFF;
        }

        // 根据不同格式写入消息头部信息
        if (fmt == kRtmpFmt0)
        {
            // 格式0：写入时间戳、消息长度、消息类型和消息流ID
            p += BytesWriter::WriteUint24T(p, ts);
            p += BytesWriter::WriteUint24T(p, h->msg_len);
            p += BytesWriter::WriteUint8T(p, h->msg_type);

            memcpy(p, &h->msg_sid, 4);
            p += 4;

            // 重置增量
            out_deltas_[h->cs_id] = 0;
        }
        else if (fmt == kRtmpFmt1)
        {
            // 格式1：写入时间戳、消息长度和消息类型
            p += BytesWriter::WriteUint24T(p, ts);
            p += BytesWriter::WriteUint24T(p, h->msg_len);
            p += BytesWriter::WriteUint8T(p, h->msg_type);
            out_deltas_[h->cs_id] = timestamp;
        }
        else if (fmt == kRtmpFmt2)
        {
            // 格式2：仅写入时间戳
            p += BytesWriter::WriteUint24T(p, ts);
            // 更新增量
            out_deltas_[h->cs_id] = timestamp;
        }

        // 如果时间戳达到最大值，写入扩展时间戳
        if (ts == 0xFFFFFF)
        {
            memcpy(p, &timestamp, 4);
            p += 4;
        }

        // 将构建好的消息头部数据保存到发送缓冲区
        BufferNodePtr nheader = std::make_shared<BufferNode>(out_current_, p - out_current_);
        sending_bufs_.emplace_back(std::move(nheader));
        out_current_ = p;

        // 更新上次的消息头信息
        prev->cs_id = h->cs_id;
        prev->msg_len = h->msg_len;
        prev->msg_sid = h->msg_sid;
        prev->msg_type = h->msg_type;

        // 如果使用格式0，直接更新时间戳；否则，增量更新
        if (fmt == kRtmpFmt0)
        {
            prev->timestamp = timestamp;
        }
        else
        {
            prev->timestamp += timestamp;
        }

        // 处理消息体部分，将数据分块并添加到发送队列中
        // 指向消息体数据的起始位置
        const char *body = packet->Data();
        // 用于跟踪已经处理的数据字节数
        int32_t bytes_parsed = 0;

        // 持续处理消息体数据，直到所有数据被分块处理完毕
        while (true)
        {
            // 计算每次发送的数据块大小
            // 指向当前要处理的消息体数据的开始位置
            const char *chunk = body + bytes_parsed;
            // 表示当前块的大小，等于剩余消息体长度和设定的最大块大小（out_chunk_size_）之间的较小值
            int32_t size = h->msg_len - bytes_parsed;
            size = std::min(size, out_chunk_size_);

            // 创建一个新的 BufferNode 对象，将当前块数据封装进去，并添加到 sending_bufs_
            // 缓冲区队列中
            BufferNodePtr node = std::make_shared<BufferNode>((void *)chunk, size);
            sending_bufs_.emplace_back(std::move(node));
            // 更新 bytes_parsed，以记录已经处理的数据量
            bytes_parsed += size;

            // 如果数据还未发送完，继续处理
            if (bytes_parsed < h->msg_len)
            {
                // 如果缓冲区不够用，输出错误信息并停止处理
                if (out_current_ - out_buffer_ >= 4096)
                {
                    RTMP_ERROR << " rtmp had no enough out header buffer.";

                    break;
                }

                // 构建后续的数据块头部
                char *p = out_current_;

                // 对于不同的 cs_id 范围，分别使用不同的方式构建头部
                // 如果 Chunk Stream ID (cs_id) 小于 64
                if (h->cs_id < 64)
                {
                    // 将格式3的标志位与 cs_id 结合编码到一个字节中，并存入缓冲区，同时指针 p
                    // 向后移动一位
                    *p++ = (char)(0xC0 | h->cs_id);
                }
                // 如果 Chunk Stream ID (cs_id) 在 64 到 319 之间
                else if (h->cs_id < (64 + 256))
                {
                    // 首先将格式3的标志位和高位部分0编码到一个字节中，并存入缓冲区，同时指针 p
                    // 向后移动一位
                    *p++ = (char)(0xC0 | 0);
                    // 将 cs_id 减去 64 的结果编码到第二个字节中，并存入缓冲区，同时指针 p
                    // 向后移动一位
                    *p++ = (char)(h->cs_id - 64);
                }
                // 如果 Chunk Stream ID (cs_id) 大于等于 320
                else
                {
                    // 首先将格式3的标志位和高位部分1编码到一个字节中，并存入缓冲区，同时指针 p
                    // 向后移动一位
                    *p++ = (char)(0xC0 | 1);
                    // 计算出需要编码的 cs_id 值（减去 64）
                    uint16_t cs = h->cs_id - 64;
                    // 将这个 16 位的 cs_id 复制到缓冲区中
                    memcpy(p, &cs, sizeof(uint16_t));
                    // 指针 p 向后移动两个字节，为后续数据存储做准备
                    p += sizeof(uint16_t);
                }

                // 对于时间戳超过最大值的情况，写入扩展时间戳
                if (ts == 0xFFFFFF)
                {
                    memcpy(p, &timestamp, 4);
                    p += 4;
                }

                // 构建完头部后，将其保存到发送缓冲区，并更新 out_current_ 指针
                BufferNodePtr nheader =
                    std::make_shared<BufferNode>(out_current_, p - out_current_);
                sending_bufs_.emplace_back(std::move(nheader));
                out_current_ = p;
            }
            else
            {
                // 数据发送完毕，退出循环
                break;
            }
        }

        // 将数据包添加到正在发送的队列中
        // out_sending_packets_.emplace_back(std::move(packet));

        // 如果消息体的所有块都成功构建并添加到发送队列，返回 true 表示成功
        return true;
    }

    // 如果消息头不存在，返回失败
    return false;
}

void RtmpContext::Send()
{
    if (sending_)
    {
        return;
    }

    // 标记当前状态为正在发送
    sending_ = true;

    // 最多处理 10 个数据包
    for (int i = 0; i < 10; i++)
    {
        // 如果等待队列为空
        if (out_waiting_queue_.empty())
        {
            // 退出循环，不再处理
            break;
        }

        // 取出等待队列中的第一个数据包
        PacketPtr packet = std::move(out_waiting_queue_.front());
        // 从等待队列中移除该数据包
        out_waiting_queue_.pop_front();
        // 将数据包构建为 RTMP 块
        BuildChunk(std::move(packet));
    }

    // 将准备好的数据块通过连接发送出去
    connection_->Send(sending_bufs_);
}

bool RtmpContext::Ready() const
{
    return !sending_;
}

void RtmpContext::Play(const std::string &url)
{
    // TODO: 实现播放媒体流的逻辑
}

void RtmpContext::Publish(const std::string &url)
{
    // TODO: 实现发布媒体流的逻辑
}

bool RtmpContext::BuildChunk(PacketPtr &&packet, uint32_t timestamp, bool fmt0)
{
    // 获取数据包中的 RTMP 消息头
    RtmpMsgHeaderPtr h = packet->Ext<RtmpMsgHeader>();

    // 如果消息头存在
    if (h)
    {
        // 获取之前的消息头，用于与当前消息头进行比较
        RtmpMsgHeaderPtr &prev = out_message_headers_[h->cs_id];
        // 检查是否使用时间戳增量（非格式0，之前的消息头存在，且时间戳合法，消息流ID相同）
        bool use_delta =
            !fmt0 && prev && timestamp >= prev->timestamp && h->msg_sid == prev->msg_sid;

        // 如果之前的消息头不存在，则初始化
        if (!prev)
        {
            prev = std::make_shared<RtmpMsgHeader>();
        }

        // 默认使用格式0
        int fmt = kRtmpFmt0;

        // 如果使用时间戳增量
        if (use_delta)
        {
            // 使用格式1
            fmt = kRtmpFmt1;

            // 计算时间戳差值
            timestamp -= prev->timestamp;

            // 如果消息类型和长度相同，使用格式2
            if (h->msg_type == prev->msg_type && h->msg_len == prev->msg_len)
            {
                fmt = kRtmpFmt2;

                // 如果时间戳差值相同，使用格式3
                if (timestamp == out_deltas_[h->cs_id])
                {
                    fmt = kRtmpFmt3;
                }
            }
        }

        // 指向当前缓冲区位置的指针
        char *p = out_current_;

        // 如果 chunk stream ID 小于 64，直接使用单字节表示
        if (h->cs_id < 64)
        {
            // 将 fmt 左移 6 位，然后与 cs_id 进行按位或操作，将结果存入 p，并且 p 指针自增
            *p++ = (char)((fmt << 6) | h->cs_id);
        }
        // 如果 chunk stream ID 在 64 到 319 之间，使用两字节表示
        else if (h->cs_id < (64 + 256))
        {
            *p++ = (char)((fmt << 6) | 0); // 第一字节：fmt 左移 6 位，与 0 进行按位或操作
            *p++ = (char)(h->cs_id - 64); // 第二字节：存储 cs_id 减去 64 的值
        }
        // 如果 chunk stream ID 大于等于 320，使用三字节表示
        else
        {
            *p++ = (char)((fmt << 6) | 1); // 第一字节：fmt 左移 6 位，与 1 进行按位或操作
            uint16_t cs =
                h->cs_id - 64; // 计算 cs_id 减去 64 的值，并将其存入一个 16 位无符号整数中
            memcpy(p, &cs, sizeof(uint16_t)); // 将 cs 的内容复制到 p 所指向的位置
            p += sizeof(uint16_t);            // p 指针向前移动 2 字节
        }

        // 设置时间戳变量
        auto ts = timestamp;

        // 如果时间戳超过最大值，设置为最大值
        if (timestamp >= 0xFFFFFF)
        {
            ts = 0xFFFFFF;
        }

        // 根据格式不同，设置不同的消息头
        if (fmt == kRtmpFmt0)
        {
            // 写入24位时间戳
            p += BytesWriter::WriteUint24T(p, ts);
            // 写入消息长度
            p += BytesWriter::WriteUint24T(p, h->msg_len);
            // 写入消息类型
            p += BytesWriter::WriteUint8T(p, h->msg_type);

            // 写入消息流 ID
            memcpy(p, &h->msg_sid, 4);
            p += 4;
            // 重置增量时间戳
            out_deltas_[h->cs_id] = 0;
        }
        else if (fmt == kRtmpFmt1)
        {
            // 写入24位时间戳
            p += BytesWriter::WriteUint24T(p, ts);
            // 写入消息长度
            p += BytesWriter::WriteUint24T(p, h->msg_len);
            // 写入消息类型
            p += BytesWriter::WriteUint8T(p, h->msg_type);
            // 更新增量时间戳
            out_deltas_[h->cs_id] = timestamp;
        }
        else if (fmt == kRtmpFmt2)
        {
            // 写入24位时间戳
            p += BytesWriter::WriteUint24T(p, ts);
            // 更新增量时间戳
            out_deltas_[h->cs_id] = timestamp;
        }

        // 如果时间戳为最大值，写入完整的时间戳
        if (ts == 0xFFFFFF)
        {
            memcpy(p, &timestamp, 4);
            p += 4;
        }

        // 创建并保存数据块头部
        BufferNodePtr nheader = std::make_shared<BufferNode>(out_current_, p - out_current_);
        sending_bufs_.emplace_back(std::move(nheader));
        out_current_ = p;

        // 更新之前的消息头信息
        prev->cs_id = h->cs_id;
        prev->msg_len = h->msg_len;
        prev->msg_sid = h->msg_sid;
        prev->msg_type = h->msg_type;

        // 更新时间戳
        if (fmt == kRtmpFmt0)
        {
            prev->timestamp = timestamp;
        }
        else
        {
            prev->timestamp += timestamp;
        }

        // 处理消息体，将其分块并添加到发送缓冲区
        const char *body = packet->Data();
        int32_t bytes_parsed = 0;

        while (true)
        {
            // 获取当前数据块
            const char *chunk = body + bytes_parsed;
            // 计算剩余的消息体长度
            int32_t size = h->msg_len - bytes_parsed;
            // 计算本次发送的数据块大小
            size = std::min(size, out_chunk_size_);

            // 创建数据块节点
            BufferNodePtr node = std::make_shared<BufferNode>((void *)chunk, size);
            // 添加到发送缓冲区
            sending_bufs_.emplace_back(std::move(node));
            // 更新已解析的字节数
            bytes_parsed += size;

            // 如果还有数据未发送完
            if (bytes_parsed < h->msg_len)
            {
                // 检查缓冲区是否足够
                if (out_current_ - out_buffer_ >= 4096)
                {
                    // 缓冲区不足时输出错误信息
                    RTMP_ERROR << " rtmp had no enough out header buffer.";

                    break;
                }

                // 获取当前缓冲区指针
                char *p = out_current_;

                // 对于不同的 cs_id 范围，分别使用不同的方式构建头部
                // 如果 Chunk Stream ID (cs_id) 小于 64
                if (h->cs_id < 64)
                {
                    // 将格式3的标志位与 cs_id 结合编码到一个字节中，并存入缓冲区，同时指针 p
                    // 向后移动一位
                    *p++ = (char)(0xC0 | h->cs_id);
                }
                // 如果 Chunk Stream ID (cs_id) 在 64 到 319 之间
                else if (h->cs_id < (64 + 256))
                {
                    // 首先将格式3的标志位和高位部分0编码到一个字节中，并存入缓冲区，同时指针 p
                    // 向后移动一位
                    *p++ = (char)(0xC0 | 0);
                    // 将 cs_id 减去 64 的结果编码到第二个字节中，并存入缓冲区，同时指针 p
                    // 向后移动一位
                    *p++ = (char)(h->cs_id - 64);
                }
                // 如果 Chunk Stream ID (cs_id) 大于等于 320
                else
                {
                    // 首先将格式3的标志位和高位部分1编码到一个字节中，并存入缓冲区，同时指针 p
                    // 向后移动一位
                    *p++ = (char)(0xC0 | 1);
                    // 计算出需要编码的 cs_id 值（减去 64）
                    uint16_t cs = h->cs_id - 64;
                    // 将这个 16 位的 cs_id 复制到缓冲区中
                    memcpy(p, &cs, sizeof(uint16_t));
                    // 指针 p 向后移动两个字节，为后续数据存储做准备
                    p += sizeof(uint16_t);
                }

                // 如果时间戳为最大值，写入完整的时间戳
                if (ts == 0xFFFFFF)
                {
                    memcpy(p, &timestamp, 4);
                    p += 4;
                }

                // 创建并保存后续数据块头部
                BufferNodePtr nheader =
                    std::make_shared<BufferNode>(out_current_, p - out_current_);
                sending_bufs_.emplace_back(std::move(nheader));
                // 更新缓冲区指针
                out_current_ = p;
            }
            else
            {
                // 数据发送完毕，退出循环
                break;
            }
        }

        // 将数据包添加到正在发送的队列中
        out_sending_packets_.emplace_back(std::move(packet));

        // 构建成功，返回 true
        return true;
    }

    // 消息头不存在，返回 false
    return false;
}

void RtmpContext::CheckAndSend()
{
    sending_ = false;
    // 重置当前缓冲区指针到缓冲区的起始位置
    out_current_ = out_buffer_;
    // 清空正在发送的缓冲区
    sending_bufs_.clear();
    // 清空正在发送的数据包列表
    out_sending_packets_.clear();

    // 如果等待发送队列不为空
    if (!out_waiting_queue_.empty())
    {
        // 调用 Send() 方法开始发送数据
        Send();
    }
    else // 如果等待发送队列为空
    {
        // 检查是否存在 RTMP 处理器
        if (rtmp_handler_)
        {
            // 调用处理器的 OnActive 方法，表示当前连接活跃
            rtmp_handler_->OnActive(connection_);
        }
    }
}

void RtmpContext::PushOutQueue(PacketPtr &&packet)
{
    out_waiting_queue_.emplace_back(std::move(packet));
    Send();
}

void RtmpContext::SendSetChunkSize()
{
    // 创建一个新的数据包，初始大小为 64 字节
    PacketPtr packet = Packet::NewPacket(64);

    // 创建一个 RTMP 消息头的共享指针
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();

    // 如果消息头创建成功
    if (header)
    {
        header->cs_id = kRtmpCSIDCommand; // 设置 Chunk Stream ID 为 RTMP 命令类型的固定 ID
        header->msg_len = 0;              // 初始化消息长度为 0，稍后会设置
        header->msg_type = kRtmpMsgTypeChunkSize; // 设置消息类型为设置 Chunk Size 类型
        header->timestamp = 0;                    // 设置时间戳为 0
        header->msg_sid = kRtmpMsID0; // 设置消息 Stream ID 为 0，表示系统控制消息
        packet->SetExt(header);       // 将消息头附加到数据包中
    }

    // 获取数据包的指针
    char *body = packet->Data();

    // 将当前的 Chunk Size 写入到数据包的 body 部分
    header->msg_len = BytesWriter::WriteUint32T(body, out_chunk_size_);

    // 设置数据包的实际大小
    packet->SetPacketSize(header->msg_len);

    // 打印调试信息，输出当前发送的 Chunk Size 以及目标主机的 IP 和端口
    RTMP_DEBUG << " send chuck size : " << out_chunk_size_
               << " to host : " << connection_->PeerAddr().ToIpPort();

    // 将数据包放入发送队列，并触发发送
    PushOutQueue(std::move(packet));
}

void RtmpContext::SendAckWindowSize()
{
    // 创建一个新的数据包，初始大小为 64 字节
    PacketPtr packet = Packet::NewPacket(64);

    // 创建一个 RTMP 消息头的共享指针
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();

    // 如果消息头创建成功
    if (header)
    {
        header->cs_id = kRtmpCSIDCommand; // 设置 Chunk Stream ID 为 RTMP 命令类型的固定 ID
        header->msg_len = 0;              // 初始化消息长度为 0，稍后会设置
        header->msg_type = kRtmpMsgTypeWindowACKSize; // 设置消息类型为窗口确认大小
        header->timestamp = 0;                        // 设置时间戳为 0
        header->msg_sid = kRtmpMsID0; // 设置消息 Stream ID 为 0，表示系统控制消息
        packet->SetExt(header);       // 将消息头附加到数据包中
    }

    // 获取数据包的指针，用于写入数据
    char *body = packet->Data();

    // 将当前的确认窗口大小写入到数据包的 body 部分
    header->msg_len = BytesWriter::WriteUint32T(body, ack_size_);

    // 设置数据包的实际大小
    packet->SetPacketSize(header->msg_len);

    // 打印调试信息，输出当前发送的确认窗口大小以及目标主机的 IP 和端口
    RTMP_DEBUG << " send act size : " << ack_size_
               << " to host : " << connection_->PeerAddr().ToIpPort();

    // 将数据包放入发送队列，并触发发送
    PushOutQueue(std::move(packet));
}

void RtmpContext::SendSetPeerBandwidth()
{
    // 创建一个新的数据包，初始大小为 64 字节
    PacketPtr packet = Packet::NewPacket(64);

    // 创建一个 RTMP 消息头的共享指针
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();

    if (header) // 如果消息头创建成功
    {
        header->cs_id = kRtmpCSIDCommand; // 设置 Chunk Stream ID 为 RTMP 命令类型的固定 ID
        header->msg_len = 0;              // 初始化消息长度为 0，稍后会设置
        header->msg_type = kRtmpMsgTypeSetPeerBW; // 设置消息类型为 Set Peer Bandwidth
        header->timestamp = 0;                    // 设置时间戳为 0
        header->msg_sid = kRtmpMsID0; // 设置消息 Stream ID 为 0，表示系统控制消息
        packet->SetExt(header);       // 将消息头附加到数据包中
    }

    // 获取数据包的指针，用于写入数据
    char *body = packet->Data();

    // 将当前的确认窗口大小写入到数据包的 body 部分
    body += BytesWriter::WriteUint32T(body, ack_size_);

    // 设置限制类型，0x02 表示动态（type = 2）
    *body++ = 0x02;

    header->msg_len = 5;

    // 设置数据包的实际大小为 5 字节（4 字节的确认窗口大小 + 1 字节的限制类型）
    packet->SetPacketSize(5);

    // 打印调试信息，输出当前发送的带宽限制以及目标主机的 IP 和端口
    RTMP_DEBUG << " send bandwidth : " << ack_size_
               << " to host : " << connection_->PeerAddr().ToIpPort();

    // 将数据包放入发送队列，并触发发送
    PushOutQueue(std::move(packet));
}

void RtmpContext::SendBytesRecv()
{
    // 如果接收的字节数大于等于确认窗口大小
    if (in_bytes_ >= ack_size_)
    {
        // 创建一个新的数据包，初始大小为 64 字节
        PacketPtr packet = Packet::NewPacket(64);

        // 创建一个 RTMP 消息头的共享指针
        RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();

        // 如果消息头创建成功
        if (header)
        {
            header->cs_id = kRtmpCSIDCommand; // 设置 Chunk Stream ID 为 RTMP 命令类型的固定 ID
            header->msg_len = 0;              // 初始化消息长度为 0，稍后会设置
            header->msg_type =
                kRtmpMsgTypeBytesRead;    // 设置消息类型为 Bytes Received (BytesRead) 消息
            header->timestamp = 0;        // 设置时间戳为 0
            header->msg_sid = kRtmpMsID0; // 设置消息 Stream ID 为 0，表示系统控制消息
            packet->SetExt(header);       // 将消息头附加到数据包中
        }

        // 获取数据包的指针，用于写入数据
        char *body = packet->Data();

        // 将已接收的字节数写入到数据包的 body 部分，并更新消息长度
        header->msg_len = BytesWriter::WriteUint32T(body, in_bytes_);

        // 设置数据包的实际大小为消息长度
        packet->SetPacketSize(header->msg_len);

        // RTMP_DEBUG << " send act size : " << ack_size_ << " to host : " <<
        // connection_->PeerAddr().ToIpPort();

        // 将数据包放入发送队列，并触发发送
        PushOutQueue(std::move(packet));

        // 重置接收字节计数
        in_bytes_ = 0;
    }
}

void RtmpContext::SendUserCtrlMessage(short nType, uint32_t value1, uint32_t value2)
{
    // 创建一个新的数据包，初始大小为 64 字节
    PacketPtr packet = Packet::NewPacket(64);

    // 创建一个 RTMP 消息头的共享指针
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();

    // 如果消息头创建成功
    if (header)
    {
        header->cs_id = kRtmpCSIDCommand; // 设置 Chunk Stream ID 为 RTMP 命令类型的固定 ID
        header->msg_len = 0; // 初始化消息长度为 0，稍后会根据写入的数据内容进行设置
        header->msg_type = kRtmpMsgTypeUserControl; // 设置消息类型为 User Control Message
        header->timestamp = 0;                      // 设置时间戳为 0
        header->msg_sid = kRtmpMsID0; // 设置消息 Stream ID 为 0，表示系统控制消息
        packet->SetExt(header);       // 将消息头附加到数据包中
    }

    // 获取数据包的指针，用于写入数据
    char *body = packet->Data();
    char *p = body;

    // 写入用户控制消息类型 (nType)
    p += BytesWriter::WriteUint16T(body, nType);

    // 写入第一个值 (value1)，通常用于标识用户控制消息的相关信息
    p += BytesWriter::WriteUint32T(body, value1);

    // 如果消息类型是设置缓冲区长度，则还需要写入第二个值 (value2)
    if (nType == kRtmpEventTypeSetBufferLength)
    {
        p += BytesWriter::WriteUint32T(body, value2);
    }

    header->msg_len = p - body;

    // 设置数据包的实际大小为已写入的字节数
    packet->SetPacketSize(header->msg_len);

    // 打印调试信息，显示发送的用户控制消息的类型和值
    RTMP_DEBUG << " send user control type : " << nType << " value : " << value1
               << " value2 : " << value2 << " to host : " << connection_->PeerAddr().ToIpPort();

    // 将数据包放入发送队列，并触发发送
    PushOutQueue(std::move(packet));
}

void RtmpContext::HandleChunkSize(PacketPtr &packet)
{
    // 检查数据包的大小是否至少为 4 字节，这是一个有效 Chunk Size 消息所需的最小长度
    if (packet->PacketSize() >= 4)
    {
        // 从数据包的内容中读取 4 字节无符号整数，即 Chunk Size 的新值
        auto size = BytesReader::ReadUint32T(packet->Data());

        // 输出调试信息，显示当前的 Chunk Size 以及即将更新的新值
        RTMP_DEBUG << " recv chunk size in_chunk_size : " << in_chunk_size_
                   << " change to : " << size;

        // 更新当前的 Chunk Size 值
        in_chunk_size_ = size;
    }
    else
    {
        // 如果数据包的大小不足 4 字节，输出错误信息，提示数据包无效
        RTMP_ERROR << " invalid chunk size packet msg_len : " << packet->PacketSize()
                   << " host : " << connection_->PeerAddr().ToIpPort();
    }
}

void RtmpContext::HandleAckWindowSize(PacketPtr &packet)
{
    // 检查数据包的大小是否至少为 4 字节，这是一个有效的 Ack Window Size 消息所需的最小长度
    if (packet->PacketSize() >= 4)
    {
        // 从数据包的内容中读取 4 字节无符号整数，即 Ack Window Size 的新值
        auto size = BytesReader::ReadUint32T(packet->Data());

        // 输出调试信息，显示当前的 Ack Window Size 以及即将更新的新值
        RTMP_DEBUG << " recv ack window size ack_size_ : " << ack_size_ << " change to : " << size;

        // 更新当前的 Ack Window Size 值
        ack_size_ = size;
    }
    else
    {
        // 如果数据包的大小不足 4 字节，输出错误信息，提示数据包无效
        RTMP_ERROR << " invalid ack window size packet msg_len : " << packet->PacketSize()
                   << " host : " << connection_->PeerAddr().ToIpPort();
    }
}

void RtmpContext::HandleUserMessage(PacketPtr &packet)
{
    // 获取数据包的大小
    auto msg_len = packet->PacketSize();

    // 如果数据包的大小小于 6 字节，认为该用户控制消息无效，输出错误日志并返回
    if (msg_len < 6)
    {
        RTMP_ERROR << " invalid user control packet msg_len : " << packet->PacketSize()
                   << " host : " << connection_->PeerAddr().ToIpPort();
        return;
    }

    // 获取数据包的内容指针
    char *body = packet->Data();

    // 读取前 2 字节，作为消息类型
    auto type = BytesReader::ReadUint16T(body);

    // 读取接下来的 4 字节，作为消息值
    auto value = BytesReader::ReadUint32T(body + 2);

    // 输出接收到的用户控制消息的类型和值
    RTMP_TRACE << " recv user control type : " << type << " value : " << value
               << " host : " << connection_->PeerAddr().ToIpPort();

    // 根据消息类型进行处理
    switch (type)
    {
    // 处理 Stream Begin 消息
    case kRtmpEventTypeStreamBegin: {
        RTMP_TRACE << " recv stream begin value : " << value
                   << " host : " << connection_->PeerAddr().ToIpPort();
        break;
    }
    // 处理 Stream EOF 消息
    case kRtmpEventTypeStreamEOF: {
        RTMP_TRACE << " recv stream eof value : " << value
                   << " host : " << connection_->PeerAddr().ToIpPort();
        break;
    }
    // 处理 Stream Dry 消息
    case kRtmpEventTypeStreamDry: {
        RTMP_TRACE << " recv stream dry value : " << value
                   << " host : " << connection_->PeerAddr().ToIpPort();
        break;
    }
    // 处理 Set Buffer Length 消息
    case kRtmpEventTypeSetBufferLength: {
        RTMP_TRACE << " recv set buffer length value : " << value
                   << " host : " << connection_->PeerAddr().ToIpPort();

        // 进一步检查数据包长度是否足够，处理 Set Buffer Length 需要至少 10 字节的数据包
        if (msg_len < 10)
        {
            RTMP_ERROR << " invalid user control packet msg_len : " << packet->PacketSize()
                       << " host : " << connection_->PeerAddr().ToIpPort();
            return;
        }
        break;
    }
    // 处理 Streams Recorded 消息
    case kRtmpEventTypeStreamsRecorded: {
        RTMP_TRACE << " recv stream recorded value : " << value
                   << " host : " << connection_->PeerAddr().ToIpPort();
        break;
    }
    // 处理 Ping Request 消息
    case kRtmpEventTypePingRequest: {
        RTMP_TRACE << " recv ping request value : " << value
                   << " host : " << connection_->PeerAddr().ToIpPort();
        // 发送 Ping Response 消息
        SendUserCtrlMessage(kRtmpEventTypePingResponse, value, 0);
        break;
    }
    // 处理 Ping Response 消息
    case kRtmpEventTypePingResponse: {
        RTMP_TRACE << " recv ping response value : " << value
                   << " host : " << connection_->PeerAddr().ToIpPort();
        break;
    }
    // 默认处理，忽略未识别的用户控制消息类型
    default:
        break;
    }
}