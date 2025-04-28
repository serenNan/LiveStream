#include "RtmpServer.h"
#include "RtmpContext.h"
#include "mmedia/base/MMediaLog.h"

using namespace tmms::mm;

// RtmpServer 构造函数，初始化基类 TcpServer 和 rtmp_handler_ 成员变量
RtmpServer::RtmpServer(EventLoop *loop, const InetAddress &local, RtmpHandler *handler)
    : TcpServer(loop, local) // 调用 TcpServer 构造函数，初始化事件循环和本地地址
      ,
      rtmp_handler_(handler) // 初始化 rtmp_handler_，如果传入了 handler 则设置为该值
{
}

void RtmpServer::Start()
{
    // 设置各种回调函数
    // 1. 连接生命周期相关回调
    TcpServer::SetNewConnectionCallback(
        std::bind(&RtmpServer::OnNewConnection, this, std::placeholders::_1));
    TcpServer::SetDestroyConnectionCallback(
        std::bind(&RtmpServer::OnDestroyed, this, std::placeholders::_1));

    // 2. 数据传输相关回调
    TcpServer::SetMessageCallback(
        std::bind(&RtmpServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
    TcpServer::SetWriteCompleteCallback(
        std::bind(&RtmpServer::OnWriteComplete, this, std::placeholders::_1));

    // 3. 连接状态相关回调
    TcpServer::SetActiveCallback(std::bind(&RtmpServer::OnActive, this, std::placeholders::_1));

    // 启动服务器
    TcpServer::Start();

    RTMP_DEBUG << " RtmpServer Start.";
}

void RtmpServer::Stop()
{
    // 调用基类的 Stop 方法，停止服务器
    TcpServer::Stop();
}

void RtmpServer::OnNewConnection(const TcpConnectionPtr &conn)
{
    // 1. 处理自定义 RTMP 处理器的回调
    if (rtmp_handler_)
    {
        rtmp_handler_->OnNewConnection(conn);
    }

    // 2. 创建 RTMP 上下文
    RtmpContextPtr shake = std::make_shared<RtmpContext>(conn, rtmp_handler_);

    // 3. 设置连接上下文并启动握手
    conn->SetContext(kRtmpContext, shake);
    shake->StartHandShake();
}

void RtmpServer::OnDestroyed(const TcpConnectionPtr &conn)
{
    // 1. 通知 RTMP 处理器连接销毁
    if (rtmp_handler_)
    {
        rtmp_handler_->OnConnectionDestroy(conn);
    }

    // 2. 清理连接相关资源
    conn->ClearContext(kRtmpContext);
}

void RtmpServer::OnMessage(const TcpConnectionPtr &conn, MsgBuffer &buff)
{
    // 1. 获取连接的 RTMP 上下文
    RtmpContextPtr shake = conn->GetContext<RtmpContext>(kRtmpContext);

    // 2. 处理 RTMP 消息
    if (shake)
    {
        // 解析 RTMP 消息
        int ret = shake->Parse(buff);

        // 3. 处理解析结果
        if (ret == 0)
        {
            // 握手成功，记录日志
            RTMP_TRACE << " host : " << conn->PeerAddr().ToIpPort() << " handshake success.";
        }
        else if (ret == -1)
        {
            // 握手失败，强制关闭连接
            conn->ForceClose();
        }
    }
}

void RtmpServer::OnWriteComplete(const ConnectionPtr &conn)
{
    // 1. 获取 RTMP 上下文
    RtmpContextPtr shake = conn->GetContext<RtmpContext>(kRtmpContext);

    // 2. 处理写完成事件
    if (shake)
    {
        shake->OnWriteComplete();
    }
}

void RtmpServer::OnActive(const ConnectionPtr &conn)
{
    // 处理连接活跃事件
    if (rtmp_handler_)
    {
        rtmp_handler_->OnActive(conn);
    }
}

RtmpServer::~RtmpServer()
{
    // 停止服务器
    Stop();
}