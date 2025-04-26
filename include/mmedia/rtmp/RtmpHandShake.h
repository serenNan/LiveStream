#pragma once

#include "network/net/TcpConnection.h"
#include <cstdint>
#include <memory>
#include <openssl/hmac.h>
#include <openssl/sha.h>

namespace tmms
{
    using namespace tmms::network;

    namespace mm
    {
        const int kRtmpHandShakePacketSize = 1536;  ///< RTMP握手包大小常量，定义为1536字节

        enum RtmpHandShakeState
        {
            kHandShakeInit, ///< 握手初始化状态

            // client
            kHandShakePostC0C1, ///< 发送C0和C1阶段（客户端）
            kHandShakeWaitS0S1, ///< 等待接收S0和S1阶段（客户端）
            kHandShakePostC2,   ///< 发送C2阶段（客户端）
            kHandShakeWaitS2,   ///< 等待接收S2阶段（客户端）
            kHandShakeDoning,   ///< 握手进行中状态

            // server
            kHandShakeWaitC0C1, ///< 等待接收C0和C1阶段（服务端）
            kHandShakePostS0S1, ///< 发送S0和S1阶段（服务端）
            kHandShakePostS2,   ///< 发送S2阶段（服务端）
            kHandShakeWaitC2,   ///< 等待接收C2阶段（服务端）

            kHandShakeDone ///< 握手完成状态
        };

        /**
         * @brief RTMP握手协议实现类
         *
         * 实现了RTMP协议规范中的握手过程，支持简单握手和复杂握手两种模式。
         * 客户端和服务端使用相同的类，通过is_client_标志区分角色。
         * 握手流程遵循RTMP 1.0协议规范，包括：
         * - 简单握手：无加密验证
         * - 复杂握手：使用HMAC-SHA256进行加密验证
         */
        class RtmpHandShake
        {
          public:
            /**
             * @brief 构造函数
             * @param conn TCP连接智能指针，用于发送和接收握手数据
             * @param client 是否为客户端，默认为false（服务端）
             *
             * 初始化RTMP握手处理器，设置连接对象和角色（客户端或服务端）。
             */
            RtmpHandShake(const TcpConnectionPtr &conn, bool client = false);

            /**
             * @brief 开始握手过程
             *
             * 根据当前角色（客户端或服务端）生成初始握手包并发送，
             * 或准备接收对方的握手包。客户端主动发送C0C1，
             * 服务端等待接收C0C1。
             */
            void Start();

            /**
             * @brief 处理握手逻辑
             * @param buff 接收到的消息缓冲区
             * @return int32_t 握手状态码：0表示握手完成，1表示继续握手，-1表示握手失败
             *
             * 根据当前握手状态和接收到的数据处理握手流程。
             * 会验证接收到的握手包，并根据需要发送对应的响应包。
             */
            int32_t HandShake(MsgBuffer &buff);

            /**
             * @brief 写操作完成回调
             *
             * 当握手包发送完成后被调用，用于更新握手状态和进行下一步握手。
             * 根据当前状态决定接下来的握手步骤。
             */
            void WriteComplete();

            /**
             * @brief 默认析构函数
             */
            ~RtmpHandShake() = default;

          private:
            /**
             * @brief 生成随机字节
             * @return uint8_t 生成的随机字节
             *
             * 使用C++标准库的随机数生成器生成0-255范围内的随机字节，
             * 用于握手包中需要随机数据的部分。
             */
            uint8_t GenRandom();

            /**
             * @brief 创建C1或S1握手包
             *
             * 根据当前角色（客户端或服务端）生成C1或S1握手包。
             * 在复杂握手模式下，会计算HMAC-SHA256签名并嵌入到包中。
             */
            void CreateC1S1();

            /**
             * @brief 检查C1或S1握手包的正确性
             * @param data 接收到的数据指针
             * @param bytes 数据长度
             * @return int32_t HMAC签名的偏移量（>=0表示验证成功）或错误码（<0）
             *
             * 验证接收到的C1或S1握手包是否符合规范。
             * 对于复杂握手，还会验证HMAC签名的正确性。
             */
            int32_t CheckC1S1(const char *data, int bytes);

            /**
             * @brief 发送C1或S1握手包
             *
             * 通过TCP连接发送已生成的C1或S1握手包。
             */
            void SendC1S1();

            /**
             * @brief 创建C2或S2握手包
             * @param data 对方发送的S1或C1握手包数据
             * @param bytes 数据长度
             * @param offset 签名在数据包中的偏移位置
             *
             * 根据接收到的S1或C1握手包，生成对应的C2或S2握手包。
             * 在复杂握手模式下，会使用对方的签名计算新的HMAC签名。
             */
            void CreateC2S2(const char *data, int bytes, int offset);

            /**
             * @brief 发送C2或S2握手包
             *
             * 通过TCP连接发送已生成的C2或S2握手包。
             */
            void SendC2S2();

            /**
             * @brief 检查C2或S2握手包的正确性
             * @param data 接收到的数据指针
             * @param bytes 数据长度
             * @return bool 验证是否通过
             *
             * 验证接收到的C2或S2握手包是否符合规范。
             * 当前实现始终返回true，实际应用中可扩展此函数进行更严格验证。
             */
            bool CheckC2S2(const char *data, int bytes);

            TcpConnectionPtr connection_; ///< TCP连接对象指针，用于发送和接收握手数据
            bool is_client_{false}; ///< 标识当前实例是客户端还是服务端，true表示客户端
            bool is_complex_handshake_{
                true}; ///< 是否使用复杂握手模式，默认为true启用HMAC-SHA256验证
            uint8_t digest_[SHA256_DIGEST_LENGTH]; ///< 存储HMAC-SHA256签名结果的缓冲区
            uint8_t C1S1_[kRtmpHandShakePacketSize + 1]; ///< C1或S1握手包缓冲区，大小为1537字节
            uint8_t C2S2_[kRtmpHandShakePacketSize]; ///< C2或S2握手包缓冲区，大小为1536字节
            int32_t state_{kHandShakeInit}; ///< 当前握手状态，初始为kHandShakeInit
        };

        /**
         * @brief RtmpHandShake类的智能指针类型别名
         *
         * 使用std::shared_ptr管理RtmpHandShake对象的生命周期。
         */
        using RtmpHandShakePtr = std::shared_ptr<RtmpHandShake>;
    } // namespace mm
} // namespace tmms