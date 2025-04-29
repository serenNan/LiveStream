# Reactor模式实现详解

## 一、基本概念

### 1.1 为什么需要Reactor模式

#### 1.1.1 传统模型的局限性
- **并发和I/O效率问题**
  - 传统的多线程和多进程模型在处理大量并发连接时性能瓶颈
  - 频繁的上下文切换和资源分配开销大
  - I/O操作频繁导致性能下降

- **事件处理的复杂性**
  - I/O多路复用模型缺乏组织和结构
  - 事件分发和处理逻辑混杂
  - 代码维护和扩展困难

#### 1.1.2 Reactor模式的优势
- **高效的事件处理**
  - 采用事件驱动方法
  - 结合同步I/O多路复用技术(select/poll/epoll)
  - 提供不同于传统线程模型的并发处理机制

- **结构化的设计**
  - 清晰的框架处理并发I/O事件
  - 简化事件驱动程序的开发
  - 提高代码可维护性

### 1.2 Reactor模式定义
Reactor模式是一种事件驱动的设计模式，用于高效处理多个并发I/O事件。它使用一个中心化的处理器(Reactor)来监控所有的I/O请求，当I/O事件发生时，将其分派给对应的处理程序。

### 1.3 核心组件

#### 1.3.1 Handles(句柄)
- **定义**: 对操作系统资源的引用，通常是文件描述符
- **用途**: 标识网络连接或其他I/O资源
- **示例**: 客户端连接的套接字文件描述符
- **实现**: 在代码中通过`fd_`成员变量表示

#### 1.3.2 Synchronous Event Demultiplexer(事件多路分发器)
- **定义**: 等待多个句柄上事件发生的组件
- **实现**: Linux中通过select/poll/epoll实现
- **功能**: 监控多个句柄的事件(可读/可写)
- **代码体现**: EventLoop中的epoll实现

#### 1.3.3 Event Handler(事件处理器)
- **定义**: 定义事件处理接口的抽象概念
- **方法**: 包含处理各种事件的虚函数
- **类型**: 读取、写入、错误处理等
- **实现**: Event基类中的虚函数定义

#### 1.3.4 Concrete Event Handler(具体事件处理器)
- **定义**: 实现事件处理器接口的具体类
- **职责**: 提供实际的事件处理逻辑
- **示例**: 处理客户端连接、数据读写等
- **关联**: 与特定的文件描述符绑定

#### 1.3.5 Initiation Dispatcher(初始化分发器)
- **定义**: Reactor模式的核心组件
- **职责**: 
  - 管理事件循环
  - 监听和分发事件
  - 调用对应的事件处理器
- **实现**: EventLoop类

1. **Reactor（反应器）**
   - 负责监听和分发事件
   - 管理Event和EventHandler的关系
   - 实现类：`EventLoop`

2. **Event（事件）**
   - 表示I/O事件
   - 包含事件类型和文件描述符
   - 实现类：`Event`基类

3. **EventHandler（事件处理器）**
   - 实际处理事件的接口
   - 通过回调函数实现
   - 实现：通过虚函数在Event类中定义

## 二、代码实现

### 2.1 Event基类设计
```cpp
class Event : public std::enable_shared_from_this<Event> {
    friend class EventLoop;
public:
    Event(EventLoop *loop, int fd);
    virtual ~Event();

    // 事件处理虚函数
    virtual void OnRead() {};
    virtual void OnWrite() {};
    virtual void OnClose() {};
    virtual void OnError(const std::string &message) {};

    // 事件控制
    bool EnableReading(bool enable);
    bool EnableWriting(bool enable);
    int Fd() const;

protected:
    EventLoop *loop_{nullptr};  // 所属的事件循环
    int fd_{-1};               // 文件描述符
    int events_{0};            // 关注的事件标志
};
```

### 2.2 EventLoop实现
```cpp
class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    void Loop() {
        lopping_ = true;
        while (lopping_) {
            // 1. 等待事件发生
            int ret = ::epoll_wait(epoll_fd_, &epoll_events_[0],
                                 static_cast<int>(epoll_events_.size()), timeout);

            // 2. 事件分发
            for (int i = 0; i < ret; ++i) {
                struct epoll_event &ev = epoll_events_[i];
                auto iter = events_.find(ev.data.fd);
                EventPtr &event = iter->second;

                // 3. 根据事件类型调用对应处理函数
                if (ev.events & EPOLLERR) {
                    event->OnError(strerror(errno));
                }
                else if ((ev.events & EPOLLHUP) && !(ev.events & EPOLLIN)) {
                    event->OnClose();
                }
                else if (ev.events & (EPOLLIN | EPOLLPRI)) {
                    event->OnRead();
                }
                else if (ev.events & EPOLLOUT) {
                    event->OnWrite();
                }
            }

            // 4. 执行其他任务
            RunFunctions();
            wheel_.OnTimer(tmms::base::TTime::NowMS());
        }
    }

private:
    bool lopping_{false};                      // 循环标志
    int epoll_fd_{-1};                         // epoll文件描述符
    std::vector<struct epoll_event> epoll_events_; // epoll事件数组
    std::unordered_map<int, EventPtr> events_;     // 事件映射表
    std::queue<Func> functions_;                   // 待执行函数队列
    std::mutex lock_;                              // 互斥锁
    TimingWheel wheel_;                           // 定时器
};
```

### 2.3 多Reactor模式实现

#### 2.3.1 EventLoopThread
```cpp
class EventLoopThread : public base::NonCopyable {
public:
    EventLoopThread();
    ~EventLoopThread();

    void Run();
    EventLoop *Loop() const;

private:
    void StartEventLoop();
    
    EventLoop *loop_{nullptr};          // 事件循环指针
    std::thread thread_;                // 事件循环线程
    bool running_{false};               // 运行状态
    std::mutex lock_;                   // 互斥锁
    std::condition_variable condition_; // 条件变量
    std::once_flag once_;              // 一次性标志
    std::promise<int> promise_loop_;    // 线程同步
};
```

#### 2.3.2 EventLoopThreadPool
```cpp
class EventLoopThreadPool : public base::NonCopyable {
public:
    EventLoopThreadPool(int thread_num, int start = 0, int cpus = 4);
    ~EventLoopThreadPool();

    // 获取下一个事件循环（轮询方式）
    EventLoop *GetNextLoop() {
        int index = loop_index_;
        loop_index_++;
        return threads_[index % threads_.size()]->Loop();
    }

    void Start() {
        for (auto &t : threads_) {
            t->Run();
        }
    }

private:
    std::vector<EventLoopThreadPtr> threads_;    // 线程池
    std::atomic_int32_t loop_index_{0};         // 轮询索引
};
```

## 三、技术特点

### 3.1 事件驱动机制
1. **epoll实现**
   - 使用EPOLLET（边缘触发）
   - 非阻塞I/O操作
   - 高效的事件监听

2. **事件分类**
   - EPOLLIN：读事件
   - EPOLLOUT：写事件
   - EPOLLERR：错误事件
   - EPOLLHUP：连接断开

### 3.2 多线程支持
1. **线程安全**
   - 使用互斥锁保护共享资源
   - 条件变量实现线程同步
   - 原子操作保证线程安全

2. **CPU亲和性**
   ```cpp
   void bind_cpu(std::thread &t, int n) {
       cpu_set_t cpu;
       CPU_ZERO(&cpu);
       CPU_SET(n, &cpu);
       pthread_setaffinity_np(t.native_handle(), sizeof(cpu), &cpu);
   }
   ```

### 3.3 性能优化
1. **事件容器动态扩容**
   ```cpp
   if (ret == epoll_events_.size()) {
       epoll_events_.resize(epoll_events_.size() * 2);
   }
   ```

2. **智能指针管理资源**
   - 使用shared_ptr管理事件对象
   - 避免内存泄漏
   - 自动资源回收

## 四、应用场景

### 4.1 网络服务器
1. **TCP服务器**
   - 接受新连接
   - 处理客户端请求
   - 管理连接生命周期

2. **定时器服务**
   - 定时任务处理
   - 超时检测
   - 周期性任务

### 4.2 高并发处理
1. **多Reactor优势**
   - 充分利用多核CPU
   - 提高并发处理能力
   - 负载均衡

2. **线程池管理**
   - 控制线程数量
   - 任务分配策略
   - 资源利用优化

## 五、注意事项

### 5.1 异常处理
1. **错误检查**
   - epoll_wait返回值检查
   - 系统调用错误处理
   - 资源释放保证

2. **异常安全**
   - RAII原则
   - 异常传播控制
   - 状态一致性维护

### 5.2 资源管理
1. **文件描述符**
   - 及时关闭
   - 避免泄漏
   - 状态检查

2. **内存管理**
   - 智能指针使用
   - 避免循环引用
   - 合理的生命周期

## 六、优化建议

### 6.1 性能优化
1. **减少锁竞争**
   - 使用无锁数据结构
   - 细粒度锁
   - 读写锁分离

2. **内存优化**
   - 内存池
   - 对象复用
   - 零拷贝技术

### 6.2 可靠性优化
1. **错误恢复**
   - 优雅退出
   - 状态恢复
   - 日志记录

2. **监控告警**
   - 性能监控
   - 错误统计
   - 资源使用率

## 七、边缘触发模式详解

### 8.1 水平触发vs边缘触发
1. **水平触发(LT, Level Trigger)**
   - 只要文件描述符就绪，就会触发通知
   - 如果不处理，下次还会通知
   - 相对宽松，但可能会频繁触发
   - 类似于电平信号，高电平期间持续触发

2. **边缘触发(ET, Edge Trigger)**
   - 仅在状态发生变化时触发一次通知
   - 必须一次性读/写完所有数据
   - 效率更高，但要求更严格
   - 类似于脉冲信号，只在变化瞬间触发

### 8.2 边缘触发的使用要点

1. **非阻塞设置**
对应实现：[EventLoop.cpp](../../src/network/net/EventLoop.cpp)
```cpp
// 设置文件描述符为非阻塞模式
int flags = fcntl(fd, F_GETFL, 0);
fcntl(fd, F_SETFL, flags | O_NONBLOCK);
```

2. **循环读取**
对应实现：[TcpConnection.cpp](../../src/network/net/TcpConnection.cpp#L61)
```cpp
void OnRead() {
    char buffer[1024];
    while (true) {  // 必须循环读取直到出错
        ssize_t n = read(fd_, buffer, sizeof(buffer));
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 数据已经读完
                break;
            }
            // 处理其他错误
            OnError(strerror(errno));
            break;
        } else if (n == 0) {
            // 连接关闭
            OnClose();
            break;
        }
        // 处理读取的数据
        ProcessData(buffer, n);
    }
}
```

3. **完整写入**
对应实现：[TcpConnection.cpp](../../src/network/net/TcpConnection.cpp#L116)
```cpp
void OnWrite() {
    while (!write_buffer_.empty()) {
        ssize_t n = write(fd_, write_buffer_.data(), write_buffer_.size());
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 写缓冲区已满，等待下次写事件
                return;
            }
            // 处理其他错误
            OnError(strerror(errno));
            return;
        }
        write_buffer_.erase(0, n);
    }
    // 数据发送完毕，关闭写事件监听
    EnableWriting(false);
}
```

### 8.3 完整的边缘触发示例

对应实现：
- [EventLoop.cpp](../../src/network/net/EventLoop.cpp)
- [TcpConnection.cpp](../../src/network/net/TcpConnection.cpp)
- [UdpSocket.cpp](../../src/network/net/UdpSocket.cpp)

```cpp
class EpollServer {
public:
    EpollServer(int port) {
        // 创建监听socket
        listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        
        // 设置非阻塞
        SetNonBlocking(listen_fd_);
        
        // 绑定地址
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        bind(listen_fd_, (struct sockaddr*)&addr, sizeof(addr));
        
        // 开始监听
        listen(listen_fd_, SOMAXCONN);
        
        // 创建epoll实例
        epoll_fd_ = epoll_create1(0);
        
        // 添加监听socket到epoll
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;  // 使用ET模式
        ev.data.fd = listen_fd_;
        epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &ev);
    }
    
    void Run() {
        std::vector<epoll_event> events(16);
        
        while (true) {
            int nfds = epoll_wait(epoll_fd_, events.data(), events.size(), -1);
            
            for (int i = 0; i < nfds; ++i) {
                if (events[i].data.fd == listen_fd_) {
                    HandleAccept();
                } else {
                    if (events[i].events & EPOLLIN) {
                        HandleRead(events[i].data.fd);
                    }
                    if (events[i].events & EPOLLOUT) {
                        HandleWrite(events[i].data.fd);
                    }
                }
            }
            
            // 动态扩容
            if (nfds == events.size()) {
                events.resize(events.size() * 2);
            }
        }
    }

private:
    void SetNonBlocking(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
    
    void HandleAccept() {
        while (true) {  // ET模式下需要循环接受所有连接
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            int client_fd = accept(listen_fd_, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // 所有连接都已接受
                    break;
                }
                // 处理其他错误
                break;
            }
            
            // 设置新连接为非阻塞
            SetNonBlocking(client_fd);
            
            // 添加到epoll监听
            struct epoll_event ev;
            ev.events = EPOLLIN | EPOLLET;  // 使用ET模式
            ev.data.fd = client_fd;
            epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev);
        }
    }
    
    void HandleRead(int fd) {
        char buffer[4096];
        while (true) {  // ET模式下需要循环读取直到EAGAIN
            ssize_t n = read(fd, buffer, sizeof(buffer));
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // 数据读取完毕
                    break;
                }
                // 处理其他错误
                close(fd);
                break;
            } else if (n == 0) {
                // 连接关闭
                close(fd);
                break;
            }
            // 处理数据...
            ProcessData(fd, buffer, n);
        }
    }
    
    void HandleWrite(int fd) {
        // 类似HandleRead，需要循环写入直到EAGAIN
        while (!write_buffers_[fd].empty()) {
            ssize_t n = write(fd, write_buffers_[fd].data(), write_buffers_[fd].size());
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // 发送缓冲区已满，等待下次写事件
                    return;
                }
                // 处理错误
                close(fd);
                return;
            }
            write_buffers_[fd].erase(0, n);
        }
        
        // 数据全部发送完毕，关闭写事件监听
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = fd;
        epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
    }
    
    int listen_fd_;
    int epoll_fd_;
    std::unordered_map<int, std::string> write_buffers_;  // 每个连接的写缓冲区
};
```

### 8.4 边缘触发的优缺点

1. **优点**
   - 减少系统调用次数，提高效率
   - 避免重复通知，降低系统开销
   - 适合高并发场景
   - 能更好地利用系统资源

2. **缺点**
   - 编程复杂度增加
   - 必须严格按照规范编写代码
   - 容易出现饿死情况
   - 对错误处理要求更高

3. **使用建议**
   - 确保正确设置非阻塞模式
   - 实现完整的循环读写
   - 合理处理EAGAIN等错误
   - 注意避免事件饿死
   - 必要时配合定时器使用

### 8.5 在流媒体直播系统中的应用

对应实现：
- [TcpConnection.cpp](../../src/network/net/TcpConnection.cpp)
- [UdpSocket.cpp](../../src/network/net/UdpSocket.cpp)

1. **为什么流媒体系统适合ET模式**
   - 数据流特点：持续性、大块数据传输
   - 实时性要求：低延迟、高吞吐量
   - 连接特性：长连接、数据量大
   - 资源利用：需要高效处理大量并发连接

2. **流媒体场景的特殊优化**
```cpp
class StreamServer : public EpollServer {
private:
    // 针对流媒体优化的缓冲区大小
    static const size_t STREAM_BUFFER_SIZE = 128 * 1024;  // 128KB
    
    void HandleStreamRead(int fd) {
        char buffer[STREAM_BUFFER_SIZE];
        ssize_t total_read = 0;
        
        while (true) {
            ssize_t n = read(fd, buffer, STREAM_BUFFER_SIZE);
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // 所有数据都读完了
                    break;
                }
                if (errno == EINTR) {
                    // 被信号中断，继续读取
                    continue;
                }
                // 处理其他错误
                OnError(fd, strerror(errno));
                break;
            } else if (n == 0) {
                // 推流端断开连接
                OnStreamClose(fd);
                break;
            }
            
            total_read += n;
            // 处理媒体数据
            ProcessMediaData(fd, buffer, n);
            
            // 可选：流量控制
            if (total_read >= MAX_BURST_READ) {
                // 避免单个连接占用过多CPU时间
                EnableReading(fd, true);  // 保持读事件监听
                break;
            }
        }
    }
    
    void ProcessMediaData(int fd, const char* data, size_t len) {
        // 1. 解析媒体数据包
        MediaPacket packet(data, len);
        
        // 2. 根据媒体类型分别处理
        switch (packet.type) {
            case MEDIA_TYPE_VIDEO:
                ProcessVideoFrame(packet);
                break;
            case MEDIA_TYPE_AUDIO:
                ProcessAudioFrame(packet);
                break;
            case MEDIA_TYPE_METADATA:
                ProcessMetadata(packet);
                break;
        }
        
        // 3. 转发给观众端
        BroadcastToViewers(fd, packet);
    }
    
    void HandleStreamWrite(int fd) {
        auto& buffer = stream_buffers_[fd];
        while (!buffer.empty()) {
            // 使用writev批量发送多个数据块
            const size_t MAX_IOVEC = 8;  // 最大批量发送块数
            struct iovec iov[MAX_IOVEC];
            size_t iov_count = 0;
            
            // 准备多个数据块
            for (const auto& chunk : buffer) {
                if (iov_count >= MAX_IOVEC) break;
                iov[iov_count].iov_base = chunk.data();
                iov[iov_count].iov_len = chunk.size();
                iov_count++;
            }
            
            // 批量发送
            ssize_t n = writev(fd, iov, iov_count);
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // 发送缓冲区已满，等待下次写事件
                    return;
                }
                // 处理错误
                OnError(fd, strerror(errno));
                return;
            }
            
            // 更新缓冲区
            RemoveSentData(buffer, n);
        }
        
        // 数据全部发送完毕，关闭写事件监听
        EnableWriting(fd, false);
    }
};
```

3. **流媒体系统的ET模式优化策略**
   - **缓冲区管理**
     - 使用更大的缓冲区(如128KB)
     - 实现零拷贝机制
     - 使用内存池避免频繁分配

   - **数据包处理**
     - 批量处理数据包
     - 实现数据包的优先级队列
     - 支持关键帧优先发送

   - **流量控制**
     - 实现推流端限速
     - 观众端带宽自适应
     - 高低延迟策略切换

   - **资源管理**
     - 推流端优先级高于观众端
     - 实现智能负载均衡
     - 动态调整缓冲区大小

4. **注意事项**
   - 推流端掉线处理要及时
   - 观众端卡顿需要及时清理积压数据
   - 关键帧丢失要处理好
   - 网络抖动要有容错机制
   - 要有完善的监控告警
```