#include "RtmpHandShake.h"
#include "base/TTime.h"
#include "mmedia/base/MMediaLog.h"
#include <cstdint>
#include <random>

// 根据OpenSSL版本定义不同的HMAC操作宏，以兼容不同版本的OpenSSL库
#if OPENSSL_VERSION_NUMBER > 0x10100000L
// OpenSSL 1.1.0及以上版本的HMAC操作宏
#define HMAC_setup(ctx, key, len)                                                                  \
    ctx = HMAC_CTX_new();                                                                          \
    HMAC_Init_ex(ctx, key, len, EVP_sha256(), 0)
#define HMAC_crunch(ctx, buf, len) HMAC_Update(ctx, buf, len)
#define HMAC_finish(ctx, dig, dlen)                                                                \
    HMAC_Final(ctx, dig, &dlen);                                                                   \
    HMAC_CTX_free(ctx)
#else
// OpenSSL 1.1.0以下版本的HMAC操作宏
#define HMAC_setup(ctx, key, len)                                                                  \
    HMAC_CTX_init(&ctx);                                                                           \
    HMAC_Init_ex(&ctx, key, len, EVP_sha256(), 0)
#define HMAC_crunch(ctx, buf, len) HMAC_Update(&ctx, buf, len)
#define HMAC_finish(ctx, dig, dlen)                                                                \
    HMAC_Final(&ctx, dig, &dlen);                                                                  \
    HMAC_CTX_cleanup(&ctx)
#endif

namespace
{
    // RTMP服务器标识版本号，用于复杂握手中识别服务器
    static const unsigned char rtmp_server_ver[4] = {0x0D, 0x0E, 0x0A, 0x0D};

    // RTMP客户端标识版本号，用于复杂握手中识别客户端
    static const unsigned char rtmp_client_ver[4] = {0x0C, 0x00, 0x0D, 0x0E};

// 客户端密钥部分长度，用于HMAC签名计算
#define PLAYER_KEY_OPEN_PART_LEN 30 ///< 用于第一次客户端摘要签名的部分密钥长度

    // 标准Adobe Flash Player客户端密钥，用于生成和验证HMAC签名
    // 前30个字节是明文部分，后32个字节是加密部分
    static const uint8_t rtmp_player_key[] = {
        'G',  'e',  'n',  'u',  'i',  'n',  'e',  ' ',  'A',  'd',  'o',  'b',  'e',  ' ',  'F',
        'l',  'a',  's',  'h',  ' ',  'P',  'l',  'a',  'y',  'e',  'r',  ' ',  '0',  '0',  '1',

        0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8, 0x2E, 0x00, 0xD0, 0xD1, 0x02, 0x9E, 0x7E,
        0x57, 0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB, 0x93, 0xB8, 0xE6, 0x36, 0xCF, 0xEB,
        0x31, 0xAE};

// 服务器密钥部分长度，用于HMAC签名计算
#define SERVER_KEY_OPEN_PART_LEN 36 ///< 用于第一次服务器摘要签名的部分密钥长度

    // 标准Adobe Flash Media Server服务器密钥，用于生成和验证HMAC签名
    // 前36个字节是明文部分，后32个字节是加密部分
    static const uint8_t rtmp_server_key[] = {
        'G',  'e',  'n',  'u',  'i',  'n',  'e',  ' ',  'A',  'd',  'o',  'b',
        'e',  ' ',  'F',  'l',  'a',  's',  'h',  ' ',  'M',  'e',  'd',  'i',
        'a',  ' ',  'S',  'e',  'r',  'v',  'e',  'r',  ' ',  '0',  '0',  '1',

        0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8, 0x2E, 0x00, 0xD0, 0xD1,
        0x02, 0x9E, 0x7E, 0x57, 0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB,
        0x93, 0xB8, 0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE};

    /**
     * 计算数据的HMAC-SHA256签名
     *
     * @param src 要计算签名的源数据
     * @param len 源数据长度
     * @param gap 要跳过的签名位置偏移（在此位置将插入计算结果，计算时需要跳过）
     * @param key HMAC密钥
     * @param keylen 密钥长度
     * @param dst 存储计算出的签名结果
     */
    void CalculateDigest(const uint8_t *src, int len, int gap, const uint8_t *key, int keylen,
                         uint8_t *dst)
    {
        // 存储HMAC签名的长度
        uint32_t digestLen = 0;

// 根据OpenSSL版本定义HMAC上下文类型
#if OPENSSL_VERSION_NUMBER > 0x10100000L
        HMAC_CTX *ctx; // OpenSSL 1.1.0及以上版本使用指针
#else
        HMAC_CTX ctx; // OpenSSL 1.1.0以下版本使用实例
#endif

        // 初始化HMAC上下文并设置密钥和SHA256算法
        HMAC_setup(ctx, key, keylen);

        // 如果gap<=0，表示数据中没有需要跳过的位置，直接处理整个数据块
        if (gap <= 0)
        {
            HMAC_crunch(ctx, src, len);
        }
        else
        {
            // 如果有gap，先处理gap之前的数据
            HMAC_crunch(ctx, src, gap);
            // 跳过HMAC签名所在的区域(SHA256_DIGEST_LENGTH=32字节)，处理剩余数据
            HMAC_crunch(ctx, src + gap + SHA256_DIGEST_LENGTH, len - gap - SHA256_DIGEST_LENGTH);
        }

        // 计算最终HMAC结果并释放上下文
        HMAC_finish(ctx, dst, digestLen);
    }

    /**
     * 验证数据的HMAC-SHA256签名是否正确
     *
     * @param buff 包含签名的数据缓冲区
     * @param digest_pos 签名在缓冲区中的位置
     * @param key HMAC密钥
     * @param keyLen 密钥长度
     * @return 签名是否有效
     */
    bool VerifyDigest(uint8_t *buff, int digest_pos, const uint8_t *key, size_t keyLen)
    {
        // 用于存储计算出的HMAC签名
        uint8_t digest[SHA256_DIGEST_LENGTH];

        // 计算缓冲区的HMAC签名（1536是C1/S1数据块大小）
        CalculateDigest(buff, 1536, digest_pos, key, keyLen, digest);

        // 比较计算出的签名与缓冲区中存储的签名是否一致
        return memcmp(&buff[digest_pos], digest, SHA256_DIGEST_LENGTH) == 0;
    }

    /**
     * 计算HMAC签名在数据包中的偏移位置
     * RTMP复杂握手中用于确定签名在数据包中的位置
     *
     * @param buff 数据缓冲区
     * @param off 基础偏移量
     * @param mod_val 模数值
     * @return 计算出的偏移位置
     */
    int32_t GetDigestOffset(const uint8_t *buff, int off, int mod_val)
    {
        // 最终偏移量
        uint32_t offset = 0;
        // 指向基础偏移量位置的指针
        const uint8_t *ptr = reinterpret_cast<const uint8_t *>(buff + off);
        uint32_t res;

        // 计算偏移量：将连续4个字节的值相加
        offset = ptr[0] + ptr[1] + ptr[2] + ptr[3];
        // 将计算结果对mod_val取模，再加上基础偏移量+4
        res = (offset % mod_val) + (off + 4);

        return res;
    }
} // namespace

using namespace tmms::mm;

/**
 * RtmpHandShake构造函数
 *
 * @param conn TCP连接对象
 * @param client 是否为客户端模式
 */
RtmpHandShake::RtmpHandShake(const TcpConnectionPtr &conn, bool client)
    : connection_(conn), // 初始化连接对象
      is_client_(client) // 初始化客户端标志
{
}

/**
 * 开始RTMP握手过程
 * 根据角色(客户端/服务端)初始化相应的握手状态
 */
void RtmpHandShake::Start()
{
    // 生成第一个握手数据包(C1或S1)
    CreateC1S1();

    // 客户端模式：主动发送C0+C1数据包
    if (is_client_)
    {
        state_ = kHandShakePostC0C1;
        SendC1S1();
    }
    else // 服务端模式：等待接收客户端的C0+C1数据包
    {
        state_ = kHandShakeWaitC0C1;
    }
}

/**
 * 生成0-255之间的随机字节
 * 用于填充握手数据包的随机部分
 */
uint8_t RtmpHandShake::GenRandom()
{
    std::mt19937 mt{std::random_device{}()};
    std::uniform_int_distribution<> rand(0, 256);
    return rand(mt) % 256;
}

void RtmpHandShake::CreateC1S1()
{
    // 为整个C1S1数据包(1537字节，包括1字节的C0/S0)填充随机数据
    for (int i = 0; i < kRtmpHandShakePacketSize + 1; i++)
    {
        C1S1_[i] = GenRandom();
    }

    // 设置RTMP协议版本为3(C0/S0部分)
    C1S1_[0] = '\x03';

    // 设置时间戳字段为0(C1/S1前4字节)
    memset(C1S1_ + 1, 0x00, 4);

    // 根据握手类型(简单/复杂)设置版本字段
    if (!is_complex_handshake_)
    {
        // 简单握手：版本字段为0
        memset(C1S1_ + 5, 0x00, 4);
    }
    else // 复杂握手
    {
        // 计算HMAC签名的偏移位置
        auto offset = GetDigestOffset(C1S1_ + 1, 8, 728);
        // 获取签名位置的指针
        uint8_t *data = C1S1_ + 1 + offset;

        // 根据角色设置相应的版本信息和计算HMAC签名
        if (is_client_)
        {
            // 客户端设置客户端版本标识
            memcpy(C1S1_ + 5, rtmp_client_ver, 4);

            // 使用客户端密钥计算HMAC签名
            CalculateDigest(C1S1_ + 1, kRtmpHandShakePacketSize, offset, rtmp_player_key,
                            PLAYER_KEY_OPEN_PART_LEN, data);
        }
        else // 服务端
        {
            // 服务端设置服务端版本标识
            memcpy(C1S1_ + 5, rtmp_server_ver, 4);
            // 使用服务端密钥计算HMAC签名
            CalculateDigest(C1S1_ + 1, kRtmpHandShakePacketSize, offset, rtmp_server_key,
                            SERVER_KEY_OPEN_PART_LEN, data);
        }

        // 保存计算的签名，用于后续验证
        memcpy(digest_, data, SHA256_DIGEST_LENGTH);
    }
}

/**
 * 检查接收到的C1或S1数据包是否有效
 *
 * @param data 接收到的数据
 * @param bytes 数据长度
 * @return >=0: 签名偏移位置(成功)，-1: 检查失败
 */
int32_t RtmpHandShake::CheckC1S1(const char *data, int bytes)
{
    // 检查数据包长度是否为1537字节(1字节C0/S0 + 1536字节C1/S1)
    if (bytes != 1537)
    {
        RTMP_ERROR << " unexpected C1S1, len = " << bytes;
        return -1;
    }

    // 检查协议版本是否为3
    if (data[0] != '\x03')
    {
        RTMP_ERROR << " unexpected C1S1, ver = " << data[0];
        return -1;
    }

    // 检查C1/S1中的版本字段(字节5-8)
    uint32_t *version = (uint32_t *)(data + 5);

    // 如果版本字段为0，表示是简单握手
    if (*version == 0)
    {
        is_complex_handshake_ = false;
        return 0;
    }

    // 以下处理复杂握手，计算并验证HMAC签名
    int32_t offset = -1;

    if (is_complex_handshake_)
    {
        // 指向C1/S1数据块的指针(跳过C0/S0)
        uint8_t *handshake = (uint8_t *)(data + 1);
        // 计算第一个可能的签名偏移位置
        offset = GetDigestOffset(handshake, 8, 728);

        // 根据角色验证签名
        if (is_client_)
        {
            // 客户端验证服务端S1签名
            if (!VerifyDigest(handshake, offset, rtmp_server_key, SERVER_KEY_OPEN_PART_LEN))
            {
                // 第一个位置验证失败，尝试第二个可能的偏移位置
                offset = GetDigestOffset(handshake, 772, 728);

                // 再次验证失败则返回错误
                if (!VerifyDigest(handshake, offset, rtmp_server_key, SERVER_KEY_OPEN_PART_LEN))
                {
                    return -1;
                }
            }
        }
        else // 服务端验证客户端C1签名
        {
            if (!VerifyDigest(handshake, offset, rtmp_player_key, PLAYER_KEY_OPEN_PART_LEN))
            {
                // 第一个位置验证失败，尝试第二个可能的偏移位置
                offset = GetDigestOffset(handshake, 772, 728);

                // 再次验证失败则返回错误
                if (!VerifyDigest(handshake, offset, rtmp_player_key, PLAYER_KEY_OPEN_PART_LEN))
                {
                    return -1;
                }
            }
        }
    }

    // 返回签名偏移位置或0(简单握手)
    return offset;
}

/**
 * 发送C1或S1握手数据包
 */
void RtmpHandShake::SendC1S1()
{
    // 发送C0+C1或S0+S1数据包(1537字节)
    connection_->Send((const char *)C1S1_, 1537);
}

void RtmpHandShake::CreateC2S2(const char *data, int bytes, int offset)
{
    // 为C2/S2数据包填充随机数据
    for (int i = 0; i < kRtmpHandShakePacketSize; i++)
    {
        C2S2_[i] = GenRandom();
    }

    // 将对方C1/S1数据包的时间戳部分(前8字节)复制到C2/S2中
    memcpy(C2S2_, data, 8);

    // 获取当前时间戳并设置到C2/S2的时间戳字段
    auto timestamp = tmms::base::TTime::Now();
    char *t = (char *)&timestamp;
    C2S2_[3] = t[0];
    C2S2_[2] = t[1];
    C2S2_[1] = t[2];
    C2S2_[0] = t[3];

    // 复杂握手时需要计算C2/S2的HMAC签名
    if (is_complex_handshake_)
    {
        uint8_t digest[32];

        // 根据角色使用不同的密钥计算中间密钥
        if (is_client_)
        {
            // 客户端使用S1中的签名和客户端密钥计算中间密钥
            CalculateDigest((const uint8_t *)(data + offset), 32, 0, rtmp_player_key,
                            sizeof(rtmp_player_key), digest);
        }
        else // 服务端
        {
            // 服务端使用C1中的签名和服务端密钥计算中间密钥
            CalculateDigest((const uint8_t *)(data + offset), 32, 0, rtmp_server_key,
                            sizeof(rtmp_server_key), digest);
        }

        // 使用中间密钥计算C2/S2的HMAC签名，并将结果放在C2/S2的最后32字节
        CalculateDigest(C2S2_, kRtmpHandShakePacketSize - 32, 0, digest, 32,
                        &C2S2_[kRtmpHandShakePacketSize - 32]);
    }
}

void RtmpHandShake::SendC2S2()
{
    // 发送C2或S2数据包(1536字节)
    connection_->Send((const char *)C2S2_, kRtmpHandShakePacketSize);
}

bool RtmpHandShake::CheckC2S2(const char *data, int bytes)
{
    // 当前实现简化为直接返回true
    // 完整实现应验证时间戳和HMAC签名
    return true;
}

int32_t RtmpHandShake::HandShake(MsgBuffer &buff)
{
    // 根据当前的握手状态处理数据
    switch (state_)
    {
    // 服务端等待接收客户端的C0+C1数据包
    case kHandShakeWaitC0C1: {
        // 如果数据不足1537字节，继续等待更多数据
        if (buff.ReadableBytes() < 1537)
        {
            return 1;
        }

        RTMP_TRACE << " host : " << connection_->PeerAddr().ToIpPort() << " , recv C0C1.\n";

        // 检查C0C1数据包有效性并获取签名偏移量
        auto offset = CheckC1S1(buff.Peek(), 1537);

        if (offset >= 0)
        {
            // 验证成功，创建S2数据包
            CreateC2S2(buff.Peek() + 1, 1536, offset);
            // 从缓冲区移除已处理的C0C1数据
            buff.Retrieve(1537);
            // 更新状态为准备发送S0+S1
            state_ = kHandShakePostS0S1;
            // 发送S0+S1数据包
            SendC1S1();
        }
        else
        {
            // C0C1验证失败
            RTMP_TRACE << " host : " << connection_->PeerAddr().ToIpPort()
                       << " , check C0C1 failed.\n";
            return -1;
        }
        break;
    }

    // 服务端等待接收客户端的C2数据包
    case kHandShakeWaitC2: {
        // 如果数据不足1536字节，继续等待更多数据
        if (buff.ReadableBytes() < 1536)
        {
            return 1;
        }

        RTMP_TRACE << " host : " << connection_->PeerAddr().ToIpPort() << " , recv C2.\n";

        // 检查C2数据包有效性
        if (CheckC2S2(buff.Peek(), 1536))
        {
            // C2验证成功，握手完成
            buff.Retrieve(1536);
            RTMP_TRACE << " host : " << connection_->PeerAddr().ToIpPort()
                       << " , handshake done.\n";
            state_ = kHandShakeDone;
            return 0; // 握手完成
        }
        else
        {
            // C2验证失败
            RTMP_TRACE << "host:" << connection_->PeerAddr().ToIpPort() << " , check C2 failed.\n";
            return -1;
        }
        break;
    }

    // 客户端等待接收服务端的S0+S1数据包
    case kHandShakeWaitS0S1: {
        // 如果数据不足1537字节，继续等待更多数据
        if (buff.ReadableBytes() < 1537)
        {
            return 1;
        }

        RTMP_TRACE << " host : " << connection_->PeerAddr().ToIpPort() << " , recv S0S1.\n";

        // 检查S0S1数据包有效性并获取签名偏移量
        auto offset = CheckC1S1(buff.Peek(), 1537);

        if (offset >= 0)
        {
            // 验证成功，创建C2数据包
            CreateC2S2(buff.Peek() + 1, 1536, offset);
            // 从缓冲区移除已处理的S0S1数据
            buff.Retrieve(1537);

            // 检查缓冲区是否包含S2数据包
            if (buff.ReadableBytes() == 1536) // 同时收到了S2
            {
                RTMP_TRACE << " host : " << connection_->PeerAddr().ToIpPort() << " , recv S2.\n";
                state_ = kHandShakeDoning;
                buff.Retrieve(1536); // 移除S2数据
                SendC2S2();          // 发送C2
                return 0;            // 握手完成
            }
            else
            {
                // 仅收到S0S1，更新状态为准备发送C2
                state_ = kHandShakePostC2;
                SendC2S2(); // 发送C2
            }
        }
        else
        {
            // S0S1验证失败
            RTMP_TRACE << " host : " << connection_->PeerAddr().ToIpPort()
                       << " , check S0S1 failed.\n";
            return -1;
        }
        break;
    }
    }

    // 返回1表示握手需要继续
    return 1;
}

void RtmpHandShake::WriteComplete()
{
    // 根据当前握手状态更新下一步状态
    switch (state_)
    {
    // 服务端发送S0+S1完成
    case kHandShakePostS0S1: {
        RTMP_TRACE << " host : " << connection_->PeerAddr().ToIpPort() << " , post S0S1.\n";
        // 更新状态为准备发送S2
        state_ = kHandShakePostS2;
        // 发送S2数据包
        SendC2S2();
        break;
    }

    // 服务端发送S2完成
    case kHandShakePostS2: {
        RTMP_TRACE << " host : " << connection_->PeerAddr().ToIpPort() << " , post S2.\n";
        // 更新状态为等待客户端的C2
        state_ = kHandShakeWaitC2;
        break;
    }

    // 客户端发送C0+C1完成
    case kHandShakePostC0C1: {
        RTMP_TRACE << " host : " << connection_->PeerAddr().ToIpPort() << " , post C0C1.\n";
        // 更新状态为等待服务端的S0+S1
        state_ = kHandShakeWaitS0S1;
        break;
    }

    // 客户端发送C2完成
    case kHandShakePostC2: {
        RTMP_TRACE << " host : " << connection_->PeerAddr().ToIpPort() << " , post C2 done.\n";
        // 更新状态为握手完成
        state_ = kHandShakeDone;
        break;
    }

    // 客户端握手进行中
    case kHandShakeDoning: {
        RTMP_TRACE << " host : " << connection_->PeerAddr().ToIpPort() << " , post C2 done.\n";
        // 更新状态为握手完成
        state_ = kHandShakeDone;
        break;
    }
    }
}