#pragma once
#include "mmedia/rtmp/RtmpHandler.h"
#include "network/TcpServer.h"
#include "network/net/TcpConnection.h"

namespace tmms
{
    namespace mm
    {
        using namespace tmms::network;

        /**
         * @brief RTMP 服务器类
         *
         * 该类继承自 TcpServer，用于处理 RTMP 协议的服务器端功能。
         * 负责管理 RTMP 连接、处理消息收发以及调用 RtmpHandler 处理具体的 RTMP 业务逻辑。
         */
        class RtmpServer : public TcpServer
        {
          public:
            /**
             * @brief 构造函数
             *
             * @param loop 事件循环指针
             * @param local 本地地址信息
             * @param handler RTMP 处理器指针，默认为 nullptr
             */
            RtmpServer(EventLoop *loop, const InetAddress &local, RtmpHandler *handler = nullptr);

            /**
             * @brief 启动服务器
             *
             * 重写父类的 Start 方法，启动 RTMP 服务器并开始监听连接。
             */
            void Start() override;

            /**
             * @brief 停止服务器
             *
             * 重写父类的 Stop 方法，停止 RTMP 服务器并关闭所有连接。
             */
            void Stop() override;

            /**
             * @brief 析构函数
             *
             * 清理服务器资源，确保所有连接都被正确关闭。
             */
            ~RtmpServer();

          private:
            /**
             * @brief 处理新连接
             *
             * @param conn 新建立的 TCP 连接对象指针
             */
            void OnNewConnection(const TcpConnectionPtr &conn);

            /**
             * @brief 处理连接销毁
             *
             * @param conn 被销毁的 TCP 连接对象指针
             */
            void OnDestroyed(const TcpConnectionPtr &conn);

            /**
             * @brief 处理接收到的消息
             *
             * @param conn TCP 连接对象指针
             * @param buff 消息缓冲区
             */
            void OnMessage(const TcpConnectionPtr &conn, MsgBuffer &buff);

            /**
             * @brief 处理写操作完成
             *
             * @param conn 连接对象指针
             */
            void OnWriteComplete(const ConnectionPtr &conn);

            /**
             * @brief 处理连接激活
             *
             * @param conn 连接对象指针
             */
            void OnActive(const ConnectionPtr &conn);

            RtmpHandler *rtmp_handler_{nullptr};   ///< RTMP 处理器指针
        };
    } // namespace mm
} // namespace tmmsp