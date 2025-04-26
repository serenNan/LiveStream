#include "TcpConnection.h"
#include "base/NetWork.h"
#include <cerrno>
#include <fcntl.h>
#include <memory>
#include <sys/uio.h>
#include <unistd.h>

using namespace tmms::network;

TcpConnection::TcpConnection(EventLoop *loop, int socketfd, const InetAddress &localAddr,
                             const InetAddress &peerAddr)
    : Connection(loop, socketfd, localAddr, peerAddr)
{
}

TcpConnection::~TcpConnection()
{
    OnClose();
}

void TcpConnection::SetCloseCallback(const CloseConnectionCallback &cb)
{
    close_cb_ = cb;
}

void TcpConnection::SetCloseCallback(CloseConnectionCallback &&cb)
{
    close_cb_ = std::move(cb);
}

void TcpConnection::OnClose()
{
    loop_->AssertInLoopThread();
    if (!close_)
    {
        close_ = true;
        if (close_cb_)
        {
            close_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
        }
        Event::Close();
    }
}

void TcpConnection::ForceClose()
{
    loop_->RunInLoop([this]() { OnClose(); });
}

void TcpConnection::SetRecMsgCallback(const MessageCallback &cb)
{
    message_cb_ = cb;
}

void TcpConnection::SetRecvMsgCallback(MessageCallback &&cb)
{
    message_cb_ = std::move(cb);
}

void TcpConnection::OnRead()
{
    if (close_)
    {
        NETWORK_ERROR << "host:" << peer_addr_.ToIpPort() << " had closed.";
        return;
    }

    ExtendLife();

    while (true)
    {
        int err = 0;
        auto ret = message_buffer_.ReadFd(fd_, &err);
        if (ret > 0)
        {
            if (message_cb_)
            {
                message_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()),
                            message_buffer_);
            }
        }
        else if (ret == 0)
        {
            OnClose();
            break;
        }
        else
        {
            if (err != EINTR && err != EAGAIN && err != EWOULDBLOCK)
            {
                NETWORK_ERROR << "read error:" << err;
                OnClose();
            }
            break;
        }
    }
}

void TcpConnection::OnError(const std::string &msg)
{
    NETWORK_ERROR << "host:" << peer_addr_.ToIpPort() << " error:" << msg;
    OnClose();
}

void TcpConnection::SetWriteCompleteCallback(const WriteCompleteCallback &cb)
{
    write_complete_cb_ = cb;
}

void TcpConnection::SetWriteCompleteCallback(WriteCompleteCallback &&cb)
{
    write_complete_cb_ = std::move(cb);
}

void TcpConnection::OnWrite()
{
    if (close_)
    {
        NETWORK_TRACE << "host:" << peer_addr_.ToIpPort() << " had closed.";
        return;
    }
    if (!io_vec_list_.empty())
    {
        while (true)
        {
            auto ret = ::writev(fd_, &io_vec_list_[0], io_vec_list_.size());
            if (ret >= 0)
            {
                while (ret > 0)
                {
                    if (io_vec_list_.front().iov_len > ret)
                    {
                        // 将void*转为char*后再进行指针运算
                        io_vec_list_.front().iov_base =
                            static_cast<char *>(io_vec_list_.front().iov_base) + ret;
                        io_vec_list_.front().iov_len -= ret;
                        break;
                    }
                    else
                    {
                        ret -= io_vec_list_.front().iov_len;
                        io_vec_list_.erase(io_vec_list_.begin());
                    }
                }
                if (io_vec_list_.empty())
                {
                    EnableWriting(false);
                    if (write_complete_cb_)
                    {
                        write_complete_cb_(
                            std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
                    }
                    return;
                }
            }
            else
            {
                if (errno != EINTR || errno != EAGAIN || errno != EWOULDBLOCK)
                {
                    NETWORK_ERROR << "host:" << peer_addr_.ToIpPort() << " write error:" << errno;
                    OnClose();
                    return;
                }
                break;
            }
        }
    }
    else
    {
        EnableWriting(false);
        if (write_complete_cb_)
        {
            write_complete_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
        }
    }
}

void TcpConnection::Send(std::list<BufferNodePtr> &list)
{
    loop_->RunInLoop([this, &list]() { SendInLoop(list); });
}

void TcpConnection::Send(const void *buf, size_t size)
{
    loop_->RunInLoop([this, &buf, size]() { SendInLoop(buf, size); });
}

void TcpConnection::SendInLoop(const void *buf, size_t size)
{
    if (close_)
    {
        NETWORK_ERROR << "host:" << peer_addr_.ToIpPort() << " had closed.";
        return;
    }
    size_t send_len = 0;
    if (io_vec_list_.empty())
    {
        send_len = ::write(fd_, buf, size);
        if (send_len < 0)
        {
            if (errno != EINTR || errno != EAGAIN || errno != EWOULDBLOCK)
            {
                NETWORK_ERROR << "host:" << peer_addr_.ToIpPort() << " write error:" << errno;
                OnClose();
                return;
            }
            send_len = 0;
        }
        size -= send_len;
        if (size == 0)
        {
            if (write_complete_cb_)
            {
                write_complete_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
            }
            return;
        }
    }
    if (size > 0)
    {
        struct iovec vec;
        vec.iov_base = static_cast<char *>(const_cast<void *>(buf)) + send_len;
        vec.iov_len = size;
        io_vec_list_.push_back(vec);
        EnableWriting(true);
    }
}

void TcpConnection::SendInLoop(std::list<BufferNodePtr> &list)
{
    if (close_)
    {
        NETWORK_ERROR << "host:" << peer_addr_.ToIpPort() << " had closed.";
        return;
    }
    for (auto &l : list)
    {
        struct iovec vec;
        vec.iov_base = (void *)l->addr;
        vec.iov_len = l->size;

        io_vec_list_.push_back(vec);
    }
    if (!io_vec_list_.empty())
    {
        EnableWriting(true);
    }
}

void TcpConnection::SetTimeoutCallback(int timeout, const TimeOutCallback &cb)
{
    auto cp = std::dynamic_pointer_cast<TcpConnection>(shared_from_this());

    loop_->RunAfter(timeout, [cp, &cb]() { cb(cp); });
}

void TcpConnection::SetTimeoutCallback(int timeout, TimeOutCallback &&cb)
{
    auto cp = std::dynamic_pointer_cast<TcpConnection>(shared_from_this());

    loop_->RunAfter(timeout, [cp, &cb]() { cb(cp); });
}

void TcpConnection::OnTimeout()
{
    NETWORK_ERROR << "host:" << peer_addr_.ToIpPort() << " timeout.";
    OnClose();
}

void TcpConnection::EnableCheckIdleTimeout(int32_t max_time)
{
    auto tp = std::make_shared<TimeOutEntry>(
        std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
    max_idle_time_ = max_time;
    timeout_entry_ = tp;
    loop_->InsertEntry(max_time, tp);
}

void TcpConnection::ExtendLife()
{
    auto tp = timeout_entry_.lock();
    if (tp)
    {
        loop_->InsertEntry(max_idle_time_, tp);
    }
}
