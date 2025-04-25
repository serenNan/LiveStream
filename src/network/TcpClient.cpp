#include "TcpClient.h"
#include "InetAddress.h"
#include "NetWork.h"
#include "SocketOpt.h"
#include "TcpConnection.h"
#include <asm-generic/socket.h>
#include <memory>
#include <sys/socket.h>

using namespace tmms::network;

TcpClient::TcpClient(EventLoop *loop, const InetAddress &server)
    : TcpConnection(loop, -1, InetAddress(), server), server_addr_(server)
{
}

void TcpClient::Connect()
{
    loop_->RunInLoop([this]() { ConnectInLoop(); });
}

void TcpClient::SetConnectCallback(const ConnectionCallback &cb)
{
    connection_cb_ = cb;
}

void TcpClient::SetConnectCallback(ConnectionCallback &&cb)
{
    connection_cb_ = std::move(cb);
}

void TcpClient::ConnectInLoop()
{
    loop_->AssertInLoopThread();
    fd_ = SocketOpt::CreateNonBlockingTcpSocket(AF_INET);
    if (fd_ < 0)
    {
        OnClose();
        return;
    }
    status_ = kTcpConStatusConnecting;
    loop_->AddEvent(std::dynamic_pointer_cast<TcpClient>(shared_from_this()));
    EnableWriting(true);
    //EnableCheckIdleTimeout(3);
    SocketOpt opt(fd_);
    auto ret = opt.Connect(server_addr_);
    if (ret == 0)
    {
        UpdateConnectionStatus();
        return;
    }
    else if (ret == -1)
    {
        if (errno != EINPROGRESS)
        {
            NETWORK_ERROR << " connect to server : " << server_addr_.ToIpPort()
                          << " error : " << errno;
            OnClose();
            return;
        }
    }
}

void TcpClient::UpdateConnectionStatus()
{
    status_ = kTcpConStatusConnected;
    if (connection_cb_)
    {
        connection_cb_(std::dynamic_pointer_cast<TcpClient>(shared_from_this()), true);
    }
}

bool TcpClient::CheckError()
{
    int error = 0;
    socklen_t len = sizeof(error);
    ::getsockopt(fd_, SOL_SOCKET, SO_ERROR, &error, &len);
    return error != 0;
}

void TcpClient::OnRead()
{
    if (status_ == kTcpConStatusConnecting)
    {
        if (CheckError())
        {
            NETWORK_ERROR << " connect to server : " << server_addr_.ToIpPort()
                          << " error : " << errno;
            OnClose();
            return;
        }
        UpdateConnectionStatus();

        return;
    }
    else if (status_ == kTcpConStatusConnected)
    {
        TcpConnection::OnRead();
    }
}

void TcpClient::OnWrite()
{
    if (status_ == kTcpConStatusConnecting)
    {
        if (CheckError())
        {
            NETWORK_ERROR << " connect to server : " << server_addr_.ToIpPort()
                          << " error : " << errno;
            OnClose();
            return;
        }
        UpdateConnectionStatus();

        return;
    }
    else if (status_ == kTcpConStatusConnected)
    {
        TcpConnection::OnWrite();
    }
}

void TcpClient::OnClose()
{
    if (status_ == kTcpConStatusDisConnected || status_ == kTcpConStatusConnecting)
    {
        loop_->DelEvent(std::dynamic_pointer_cast<TcpClient>(shared_from_this()));
    }
    status_ = kTcpConStatusDisConnected;
    TcpConnection::OnClose();
}

void TcpClient::Send(std::list<BufferNodePtr> &list)
{
    if (status_ == kTcpConStatusConnected)
    {
        TcpConnection::Send(list);
    }
}

void TcpClient::Send(const char *buff, size_t size)
{
    if (status_ == kTcpConStatusConnected)
    {
        TcpConnection::Send(buff, size);
    }
}

TcpClient::~TcpClient()
{
    OnClose();
}