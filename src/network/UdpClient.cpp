#include "UdpClient.h"
#include "InetAddress.h"
#include "UdpSocket.h"
#include "network/base/SocketOpt.h"
#include <sys/socket.h>

using namespace tmms::network;

UdpClient::UdpClient(EventLoop *loop, const InetAddress &server)
    : UdpSocket(loop, -1, InetAddress(), server), server_addr_(server)
{
}

void UdpClient::Connect()
{
    loop_->RunInLoop(std::bind(&UdpClient::ConnectInLoop, this));
}

void UdpClient::SetConnectedCallback(const ConnectedCallback &cb)
{
    connected_cb_ = cb;
}

void UdpClient::SetConnectedCallback(ConnectedCallback &&cb)
{
    connected_cb_ = std::move(cb);
}

void UdpClient::ConnectInLoop()
{
    loop_->AssertInLoopThread();
    fd_ = SocketOpt::CreateNonBlockingUdpSocket(AF_INET);
    if (fd_ < 0)
    {
        OnClose();
        return;
    }
    connected_ = true;
    loop_->AddEvent(std::dynamic_pointer_cast<UdpClient>(shared_from_this()));
    SocketOpt opt(fd_);
    opt.Connect(server_addr_);

    server_addr_.GetSockAddr((struct sockaddr *)&sock_addr_);

    // 启用读事件监听

    if (connected_cb_)
    {
        connected_cb_(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()), true);
    }
}

void UdpClient::Send(std::list<BufferNodePtr> &list)
{
}

void UdpClient::Send(const char *buff, size_t size)
{
    UdpSocket::Send(buff, size, (struct sockaddr *)&sock_addr_, sock_len_);
}

void UdpClient::OnClose()
{
    if (connected_)
    {
        loop_->DelEvent(std::dynamic_pointer_cast<UdpClient>(shared_from_this()));

        connected_ = false;

        UdpSocket::OnClose();
    }
}

UdpClient::~UdpClient()
{
}