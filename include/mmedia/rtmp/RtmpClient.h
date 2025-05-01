#pragma once
#include "mmedia/rtmp/RtmpHandler.h"
#include "network/TcpClient.h"
#include "network/base/InetAddress.h"
#include "network/net/EventLoop.h"
#include <functional>
#include <memory>

namespace tmms
{
    namespace mm
    {
        using namespace tmms::network;

        /**
         * @brief TcpClient的智能指针类型定义，用于管理TcpClient对象的生命周期
         */
        using TcpClientPtr = std::shared_ptr<TcpClient>;

        /**
         * @brief RTMP客户端类，负责建立RTMP连接并处理流媒体传输
         *
         * 该类封装了RTMP协议的客户端实现，可用于播放远程RTMP流或向远程服务器推送媒体流。
         * 它通过TcpClient建立网络连接，使用RtmpContext处理RTMP协议，通过RtmpHandler处理媒体数据。
         */
        class RtmpClient
        {
          public:
            /**
             * @brief 构造函数
             *
             * @param loop 事件循环指针，用于处理网络IO事件
             * @param handler RTMP协议处理器指针，用于处理RTMP数据包和媒体数据
             */
            RtmpClient(EventLoop *loop, RtmpHandler *handler);

            /**
             * @brief 设置连接关闭的回调函数
             *
             * 当RTMP连接关闭时，会调用该回调函数通知上层应用。
             *
             * @param cb 回调函数，接收一个TcpConnectionPtr参数
             */
            void SetCloseCallback(const CloseConnectionCallback &cb);

            /**
             * @brief 设置连接关闭的回调函数（移动语义版本）
             *
             * 使用移动语义优化性能，避免不必要的复制操作。
             *
             * @param cb 回调函数，通过右值引用传递
             */
            void SetCloseCallback(CloseConnectionCallback &&cb);

            /**
             * @brief 开始播放指定URL的RTMP流
             *
             * 解析URL，建立连接并开始播放流媒体内容。
             * URL格式为：rtmp://host[:port]/app/stream
             *
             * @param url RTMP流的完整URL
             */
            void Play(const std::string &url);

            /**
             * @brief 开始发布指定URL的RTMP流
             *
             * 解析URL，建立连接并准备向服务器推送流媒体数据。
             * URL格式为：rtmp://host[:port]/app/stream
             *
             * @param url 目标RTMP流的完整URL
             */
            void Publish(const std::string &url);

            /**
             * @brief 发送RTMP数据包
             *
             * 将媒体数据包通过RTMP协议发送到远程服务器。
             * 使用移动语义优化性能，适用于推流场景。
             *
             * @param data RTMP数据包的智能指针，使用右值引用传递
             */
            void Send(PacketPtr &&data);

            /**
             * @brief 析构函数，负责资源清理
             */
            ~RtmpClient();

          private:
            /**
             * @brief 写操作完成时的回调处理函数
             *
             * 当网络数据写入完成时被调用，用于处理后续的RTMP协议操作。
             *
             * @param conn TCP连接的智能指针
             */
            void OnWriteComplete(const TcpConnectionPtr &conn);

            /**
             * @brief 连接建立或断开时的回调处理函数
             *
             * 当TCP连接建立或断开时被调用，负责创建或清理RTMP上下文。
             *
             * @param conn TCP连接的智能指针
             * @param connected 连接状态，true表示已连接，false表示已断开
             */
            void OnConnection(const TcpConnectionPtr &conn, bool connected);

            /**
             * @brief 接收到消息时的回调处理函数
             *
             * 当TCP连接接收到数据时被调用，负责解析RTMP消息。
             *
             * @param conn TCP连接的智能指针
             * @param buff 接收到的消息缓冲区
             */
            void OnMessage(const TcpConnectionPtr &conn, MsgBuffer &buff);

            /**
             * @brief 解析RTMP URL
             *
             * 从URL中提取主机名、端口号等信息，用于建立TCP连接。
             *
             * @param url RTMP URL字符串
             * @return 解析是否成功，true表示成功，false表示失败
             */
            bool ParseUrl(const std::string &url);

            /**
             * @brief 创建TCP客户端并初始化连接
             *
             * 根据解析的URL创建TCP客户端，设置回调函数并发起连接。
             */
            void CreateTcpClient();

            /// 事件循环指针，用于处理网络IO事件
            EventLoop *loop_{nullptr};

            /// 存储网络地址信息，包含主机名/IP地址和端口号
            InetAddress addr_;

            /// RTMP协议处理器指针，用于处理RTMP协议相关的逻辑
            RtmpHandler *handler_{nullptr};

            /// TCP客户端的智能指针，管理TCP连接的生命周期
            TcpClientPtr tcp_client_;

            /// 保存RTMP流的完整URL
            std::string url_;

            /// 客户端模式标识，true表示播放模式，false表示发布模式
            bool is_player_{false};

            /// 存储连接关闭时的回调函数
            CloseConnectionCallback close_cb_;
        };
    } // namespace mm
} // namespace tmms