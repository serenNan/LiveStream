#include "UdpServer.h"
#include "network/base/NetWork.h"
#include "network/base/SocketOpt.h"

using namespace tmms::network;

UdpServer::UdpServer(EventLoop *loop, const InetAddress &server)
    : UdpSocket(loop, -1, server, InetAddress()), server_(server)
{
}

void UdpServer::Start()
{
    loop_->RunInLoop([this]() { Open(); });
}

void UdpServer::Stop()
{
    loop_->RunInLoop([this]() {
        loop_->DelEvent(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));
        OnClose();
    });
}

void UdpServer::Open()
{
    loop_->AssertInLoopThread();

    fd_ = SocketOpt::CreateNonBlockingUdpSocket(AF_INET);

    if (fd_ < 0)
    {
        OnClose();

        return;
    }

    loop_->AddEvent(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));

    SocketOpt opt(fd_);

    opt.BindAddress(server_);
}

UdpServer::~UdpServer()
{
    Stop();
}