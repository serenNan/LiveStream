/**
 * @brief RTMP端到端延迟测试工具
 *
 * 该程序通过同时启动推流和拉流客户端，并在推流内容中嵌入时间戳，
 * 实现对RTMP直播系统端到端延迟的精确测量
 */

#include "mmedia/rtmp/RtmpClient.h"
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThreadPool.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

using namespace tmms::network;
using namespace tmms::mm;

// 延迟样本结构
struct LatencySample
{
    uint64_t timestamp_sent;     ///< 发送时间戳(微秒)
    uint64_t timestamp_received; ///< 接收时间戳(微秒)
    uint64_t sequence_number;    ///< 序列号
    uint64_t latency_us;         ///< 延迟(微秒)
};

// 延迟统计
struct LatencyStats
{
    uint64_t min_latency_ms = UINT64_MAX; ///< 最小延迟(毫秒)
    uint64_t max_latency_ms = 0;          ///< 最大延迟(毫秒)
    uint64_t sum_latency_ms = 0;          ///< 延迟总和(毫秒)
    uint64_t sample_count = 0;            ///< 样本数量
    std::vector<uint64_t> latencies;      ///< 所有延迟样本(毫秒)

    // 计算平均延迟
    double getAverageLatency() const
    {
        if (sample_count == 0)
            return 0;
        return static_cast<double>(sum_latency_ms) / sample_count;
    }

    // 计算中位数延迟
    double getMedianLatency()
    {
        if (latencies.empty())
            return 0;

        std::sort(latencies.begin(), latencies.end());

        if (latencies.size() % 2 == 0)
        {
            // 偶数个样本，取中间两个的平均值
            size_t mid = latencies.size() / 2;
            return (latencies[mid - 1] + latencies[mid]) / 2.0;
        }
        else
        {
            // 奇数个样本，取中间值
            return latencies[latencies.size() / 2];
        }
    }

    // 计算95%百分位延迟
    uint64_t get95PercentileLatency()
    {
        if (latencies.empty())
            return 0;

        std::sort(latencies.begin(), latencies.end());
        size_t index = static_cast<size_t>(latencies.size() * 0.95);
        if (index >= latencies.size())
        {
            index = latencies.size() - 1;
        }
        return latencies[index];
    }

    // 添加延迟样本
    void addSample(uint64_t latency_ms)
    {
        min_latency_ms = std::min(min_latency_ms, latency_ms);
        max_latency_ms = std::max(max_latency_ms, latency_ms);
        sum_latency_ms += latency_ms;
        sample_count++;
        latencies.push_back(latency_ms);
    }

    // 打印统计信息
    void printStats()
    {
        if (sample_count == 0)
        {
            std::cout << "没有收集到延迟样本" << std::endl;
            return;
        }

        std::cout << "\n===== 延迟测试统计 =====" << std::endl;
        std::cout << "样本数量: " << sample_count << std::endl;
        std::cout << "最小延迟: " << min_latency_ms << " 毫秒" << std::endl;
        std::cout << "最大延迟: " << max_latency_ms << " 毫秒" << std::endl;
        std::cout << "平均延迟: " << getAverageLatency() << " 毫秒" << std::endl;
        std::cout << "中位数延迟: " << getMedianLatency() << " 毫秒" << std::endl;
        std::cout << "95%百分位延迟: " << get95PercentileLatency() << " 毫秒" << std::endl;
    }
};

// 全局延迟统计
LatencyStats g_latency_stats;

// 全局互斥锁，用于保护延迟统计数据
std::mutex g_stats_mutex;

// 标记测试是否应该继续运行
std::atomic<bool> g_running{true};

// 测试配置
struct TestConfig
{
    std::string server_ip = "127.0.0.1"; // 服务器IP
    int server_port = 1935;              // 服务器端口
    std::string app_name = "live";       // 应用名称
    std::string stream_name = "latency"; // 流名称
    int test_duration = 60;              // 测试持续时间(秒)
    int sample_interval = 1000;          // 采样间隔(毫秒)
};

// 时间戳发送队列，用于存储发送的时间戳
std::queue<uint64_t> g_timestamp_queue;
std::mutex g_queue_mutex;
std::condition_variable g_queue_cv;

// 获取当前时间戳(微秒)
uint64_t getCurrentTimestampUs()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

// RTMP推流处理器
class PublisherHandler : public RtmpHandler
{
  private:
    uint64_t seq_number_ = 0;
    std::string stream_url_;

  public:
    PublisherHandler(const std::string &stream_url) : stream_url_(stream_url)
    {
    }

    void OnNewConnection(const TcpConnectionPtr &conn) override
    {
    }
    void OnConnectionDestroy(const TcpConnectionPtr &conn) override
    {
    }
    void OnRecv(const TcpConnectionPtr &conn, const PacketPtr &data) override
    {
    }
    void OnRecv(const TcpConnectionPtr &conn, PacketPtr &&data) override
    {
    }
    void OnActive(const ConnectionPtr &conn) override
    {
    }
    bool OnPlay(const TcpConnectionPtr &conn, const std::string &session_name,
                const std::string &param) override
    {
        return true;
    }
    bool OnPublish(const TcpConnectionPtr &conn, const std::string &session_name,
                   const std::string &param) override
    {
        return true;
    }
    void OnPause(const TcpConnectionPtr &conn, bool pause) override
    {
    }
    void OnSeek(const TcpConnectionPtr &conn, double time) override
    {
    }

    // 发送带时间戳的数据包
    void SendTimestampPacket(const TcpConnectionPtr &conn)
    {
        // 在实际应用中，这里应该构造一个特殊的视频帧或元数据包
        // 简化起见，这里只是将时间戳放入队列
        uint64_t timestamp = getCurrentTimestampUs();

        {
            std::lock_guard<std::mutex> lock(g_queue_mutex);
            g_timestamp_queue.push(timestamp);
        }

        g_queue_cv.notify_one();
        seq_number_++;
    }
};

// RTMP拉流处理器
class PlayerHandler : public RtmpHandler
{
  private:
    uint64_t client_id_;
    std::string stream_url_;

  public:
    PlayerHandler(uint64_t client_id, const std::string &stream_url)
        : client_id_(client_id), stream_url_(stream_url)
    {
    }

    void OnNewConnection(const TcpConnectionPtr &conn) override
    {
    }
    void OnConnectionDestroy(const TcpConnectionPtr &conn) override
    {
    }

    // 数据接收回调，在这里提取时间戳并计算延迟
    void OnRecv(const TcpConnectionPtr &conn, const PacketPtr &data) override
    {
        ProcessPacket(data);
    }

    void OnRecv(const TcpConnectionPtr &conn, PacketPtr &&data) override
    {
        ProcessPacket(data);
    }

    void OnActive(const ConnectionPtr &conn) override
    {
    }
    bool OnPlay(const TcpConnectionPtr &conn, const std::string &session_name,
                const std::string &param) override
    {
        return true;
    }
    bool OnPublish(const TcpConnectionPtr &conn, const std::string &session_name,
                   const std::string &param) override
    {
        return false;
    }
    void OnPause(const TcpConnectionPtr &conn, bool pause) override
    {
    }
    void OnSeek(const TcpConnectionPtr &conn, double time) override
    {
    }

  private:
    void ProcessPacket(const PacketPtr &data)
    {
        // 在实际应用中，这里应该解析视频帧或元数据包，提取时间戳
        // 简化起见，这里只是从队列中取出最早的时间戳
        uint64_t sent_timestamp = 0;
        {
            std::unique_lock<std::mutex> lock(g_queue_mutex);
            if (!g_timestamp_queue.empty())
            {
                sent_timestamp = g_timestamp_queue.front();
                g_timestamp_queue.pop();
            }
            else
            {
                return; // 没有发送的时间戳可用
            }
        }

        // 计算延迟
        uint64_t now = getCurrentTimestampUs();
        uint64_t latency_us = now - sent_timestamp;
        uint64_t latency_ms = latency_us / 1000; // 转换为毫秒

        // 保存延迟样本
        {
            std::lock_guard<std::mutex> lock(g_stats_mutex);
            g_latency_stats.addSample(latency_ms);
        }

        std::cout << "延迟样本: " << latency_ms << " 毫秒" << std::endl;
    }
};

// 打印使用说明
void printUsage(const char *program)
{
    std::cout << "用法: " << program << " [选项]" << std::endl;
    std::cout << "选项:" << std::endl;
    std::cout << "  --server IP       服务器IP地址 (默认: 127.0.0.1)" << std::endl;
    std::cout << "  --port PORT       服务器端口 (默认: 1935)" << std::endl;
    std::cout << "  --app NAME        应用名称 (默认: live)" << std::endl;
    std::cout << "  --stream NAME     流名称 (默认: latency)" << std::endl;
    std::cout << "  --duration N      测试持续时间(秒) (默认: 60)" << std::endl;
    std::cout << "  --interval N      采样间隔(毫秒) (默认: 1000)" << std::endl;
}

// 解析命令行参数
bool parseArgs(int argc, const char **argv, TestConfig &config)
{
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "--server" && i + 1 < argc)
        {
            config.server_ip = argv[++i];
        }
        else if (arg == "--port" && i + 1 < argc)
        {
            config.server_port = std::stoi(argv[++i]);
        }
        else if (arg == "--app" && i + 1 < argc)
        {
            config.app_name = argv[++i];
        }
        else if (arg == "--stream" && i + 1 < argc)
        {
            config.stream_name = argv[++i];
        }
        else if (arg == "--duration" && i + 1 < argc)
        {
            config.test_duration = std::stoi(argv[++i]);
        }
        else if (arg == "--interval" && i + 1 < argc)
        {
            config.sample_interval = std::stoi(argv[++i]);
        }
        else if (arg == "--help")
        {
            return false;
        }
        else
        {
            std::cerr << "未知选项: " << arg << std::endl;
            return false;
        }
    }
    return true;
}

int main(int argc, const char **argv)
{
    // 配置参数
    TestConfig config;

    // 解析命令行参数
    if (!parseArgs(argc, argv, config))
    {
        printUsage(argv[0]);
        return 1;
    }

    std::cout << "开始 RTMP 端到端延迟测试..." << std::endl;
    std::cout << "服务器: rtmp://" << config.server_ip << ":" << config.server_port << "/"
              << config.app_name << "/" << config.stream_name << std::endl;
    std::cout << "测试持续时间: " << config.test_duration << " 秒" << std::endl;
    std::cout << "采样间隔: " << config.sample_interval << " 毫秒" << std::endl;

    // 创建线程池
    EventLoopThreadPool loop_pool(2); // 一个线程用于推流，一个用于拉流
    loop_pool.Start();

    // 创建RTMP URL
    std::string rtmp_url = "rtmp://" + config.server_ip + ":" + std::to_string(config.server_port) +
                           "/" + config.app_name + "/" + config.stream_name;

    // 创建推流客户端
    auto publisher_loop = loop_pool.GetNextLoop();
    auto publisher_handler = new PublisherHandler(rtmp_url);
    auto publisher = std::make_shared<RtmpClient>(publisher_loop, publisher_handler);

    // 启动推流
    publisher->Publish(rtmp_url);
    std::cout << "推流客户端已启动" << std::endl;

    // 等待推流连接建立
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 创建拉流客户端
    auto player_loop = loop_pool.GetNextLoop();
    auto player_handler = new PlayerHandler(1, rtmp_url);
    auto player = std::make_shared<RtmpClient>(player_loop, player_handler);

    // 启动拉流
    player->Play(rtmp_url);
    std::cout << "拉流客户端已启动" << std::endl;

    // 等待拉流连接建立
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 开始定期发送时间戳包进行延迟测量
    std::cout << "开始发送时间戳包进行延迟测量..." << std::endl;

    // 测试持续时间
    auto start_time = std::chrono::high_resolution_clock::now();

    // 每隔一段时间发送一个时间戳包
    while (g_running)
    {
        // 发送时间戳包
        publisher_handler->SendTimestampPacket(nullptr);

        // 等待指定的采样间隔
        std::this_thread::sleep_for(std::chrono::milliseconds(config.sample_interval));

        // 检查是否达到测试持续时间
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
        if (elapsed >= config.test_duration)
        {
            g_running = false;
        }
    }

    // 测试完成，输出统计信息
    std::cout << "测试完成，输出统计信息..." << std::endl;

    // 打印统计信息
    g_latency_stats.printStats();

    // 清理资源
    std::cout << "清理资源..." << std::endl;
    publisher.reset();
    player.reset();

    // 等待线程池资源释放
    std::cout << "等待所有线程完成..." << std::endl;
    // 线程池会在析构时自动清理，无需显式调用Join

    return 0;
}