/**
 * @brief RTMP流媒体系统压力测试程序
 *
 * 该程序模拟多个并发客户端，对RTMP服务器进行压力测试
 */

#include "base/TTime.h"
#include "mmedia/rtmp/RtmpClient.h"
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThreadPool.h"
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace tmms::network;
using namespace tmms::mm;
using namespace tmms::base;

// 测试配置
struct TestConfig
{
    std::string server_ip = "127.0.0.1"; ///< 服务器IP地址
    int server_port = 1935;              ///< 服务器端口
    int client_count = 100;              ///< 客户端数量
    int test_duration = 60;              ///< 测试持续时间(秒)
    std::string app_name = "live";       ///< 应用名称
    std::string stream_name = "test";    ///< 流名称
    bool play_mode = true;               ///< true为拉流测试，false为推流测试
};

// 统计信息
struct Stats
{
    std::atomic<uint64_t> connected_count{0};  ///< 成功连接数
    std::atomic<uint64_t> failed_count{0};     ///< 失败连接数
    std::atomic<uint64_t> bytes_received{0};   ///< 接收字节数
    std::atomic<uint64_t> packets_received{0}; ///< 接收包数
    std::atomic<uint64_t> bytes_sent{0};       ///< 发送字节数
    std::atomic<uint64_t> packets_sent{0};     ///< 发送包数
    std::atomic<uint64_t> latency_sum{0};      ///< 延迟总和(毫秒)
    std::atomic<uint64_t> latency_count{0};    ///< 延迟计数
};

// 全局统计信息
Stats g_stats;

// RTMP处理器实现
class BenchmarkRtmpHandler : public RtmpHandler
{
  private:
    uint64_t client_id_;
    std::string stream_url_;

  public:
    BenchmarkRtmpHandler(uint64_t client_id, const std::string &stream_url)
        : client_id_(client_id), stream_url_(stream_url)
    {
    }

    // 连接建立回调
    void OnNewConnection(const TcpConnectionPtr &conn) override
    {
        g_stats.connected_count++;
    }

    // 连接断开回调
    void OnConnectionDestroy(const TcpConnectionPtr &conn) override
    {
        // 连接断开不减计数，便于观察最大并发连接数
    }

    // 数据接收回调
    void OnRecv(const TcpConnectionPtr &conn, const PacketPtr &data) override
    {
        g_stats.bytes_received += data->PacketSize();
        g_stats.packets_received++;
    }

    // 数据接收回调 (右值版本)
    void OnRecv(const TcpConnectionPtr &conn, PacketPtr &&data) override
    {
        g_stats.bytes_received += data->PacketSize();
        g_stats.packets_received++;
    }

    // 连接激活回调
    void OnActive(const ConnectionPtr &conn) override
    {
    }

    // 播放请求回调
    bool OnPlay(const TcpConnectionPtr &conn, const std::string &session_name,
                const std::string &param) override
    {
        return true;
    }

    // 发布请求回调
    bool OnPublish(const TcpConnectionPtr &conn, const std::string &session_name,
                   const std::string &param) override
    {
        return true;
    }

    // 暂停请求回调
    void OnPause(const TcpConnectionPtr &conn, bool pause) override
    {
    }

    // 跳转请求回调
    void OnSeek(const TcpConnectionPtr &conn, double time) override
    {
    }
};

// 打印使用说明
void printUsage(const char *program)
{
    std::cout << "用法: " << program << " [选项]" << std::endl;
    std::cout << "选项:" << std::endl;
    std::cout << "  --server IP       服务器IP地址 (默认: 127.0.0.1)" << std::endl;
    std::cout << "  --port PORT       服务器端口 (默认: 1935)" << std::endl;
    std::cout << "  --clients N       客户端数量 (默认: 100)" << std::endl;
    std::cout << "  --duration N      测试持续时间(秒) (默认: 60)" << std::endl;
    std::cout << "  --app NAME        应用名称 (默认: live)" << std::endl;
    std::cout << "  --stream NAME     流名称 (默认: test)" << std::endl;
    std::cout << "  --play            拉流测试 (默认模式)" << std::endl;
    std::cout << "  --publish         推流测试" << std::endl;
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
        else if (arg == "--clients" && i + 1 < argc)
        {
            config.client_count = std::stoi(argv[++i]);
        }
        else if (arg == "--duration" && i + 1 < argc)
        {
            config.test_duration = std::stoi(argv[++i]);
        }
        else if (arg == "--app" && i + 1 < argc)
        {
            config.app_name = argv[++i];
        }
        else if (arg == "--stream" && i + 1 < argc)
        {
            config.stream_name = argv[++i];
        }
        else if (arg == "--play")
        {
            config.play_mode = true;
        }
        else if (arg == "--publish")
        {
            config.play_mode = false;
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

// 打印统计信息
void printStats(const TestConfig &config, double elapsed)
{
    std::cout << "\n====== 性能测试统计 ======" << std::endl;
    std::cout << "测试持续时间: " << elapsed << " 秒" << std::endl;
    std::cout << "目标并发客户端数: " << config.client_count << std::endl;
    std::cout << "成功连接数: " << g_stats.connected_count << std::endl;
    std::cout << "失败连接数: " << g_stats.failed_count << std::endl;

    if (config.play_mode)
    {
        // 拉流统计
        double mbits_received = (g_stats.bytes_received * 8.0) / (1024 * 1024);
        std::cout << "接收数据总量: " << g_stats.bytes_received << " 字节 (" << mbits_received
                  << " Mbits)" << std::endl;
        std::cout << "接收数据包数: " << g_stats.packets_received << std::endl;

        if (elapsed > 0)
        {
            std::cout << "接收速率: " << (mbits_received / elapsed) << " Mbps" << std::endl;
            std::cout << "数据包接收速率: " << (g_stats.packets_received / elapsed) << " 包/秒"
                      << std::endl;
        }
    }
    else
    {
        // 推流统计
        double mbits_sent = (g_stats.bytes_sent * 8.0) / (1024 * 1024);
        std::cout << "发送数据总量: " << g_stats.bytes_sent << " 字节 (" << mbits_sent << " Mbits)"
                  << std::endl;
        std::cout << "发送数据包数: " << g_stats.packets_sent << std::endl;

        if (elapsed > 0)
        {
            std::cout << "发送速率: " << (mbits_sent / elapsed) << " Mbps" << std::endl;
            std::cout << "数据包发送速率: " << (g_stats.packets_sent / elapsed) << " 包/秒"
                      << std::endl;
        }
    }

    // 如果有延迟数据，计算平均延迟
    if (g_stats.latency_count > 0)
    {
        double avg_latency = static_cast<double>(g_stats.latency_sum) / g_stats.latency_count;
        std::cout << "平均延迟: " << avg_latency << " 毫秒" << std::endl;
    }
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

    std::cout << "开始 RTMP " << (config.play_mode ? "拉流" : "推流") << " 性能测试..."
              << std::endl;
    std::cout << "服务器: rtmp://" << config.server_ip << ":" << config.server_port << "/"
              << config.app_name << "/" << config.stream_name << std::endl;
    std::cout << "客户端数量: " << config.client_count << std::endl;
    std::cout << "测试持续时间: " << config.test_duration << " 秒" << std::endl;

    // 创建线程池，用于事件循环
    const int thread_num = std::min(16, config.client_count / 10 + 1);
    EventLoopThreadPool loop_pool(thread_num);
    loop_pool.Start();

    // 存储所有客户端
    std::vector<std::shared_ptr<RtmpClient>> clients;
    clients.reserve(config.client_count);

    // 创建RTMP URL
    std::string rtmp_url = "rtmp://" + config.server_ip + ":" + std::to_string(config.server_port) +
                           "/" + config.app_name + "/" + config.stream_name;

    // 启动计时器
    auto start_time = TTime::NowMS();

    // 创建并启动所有客户端
    for (int i = 0; i < config.client_count; i++)
    {
        // 获取下一个事件循环
        EventLoop *loop = loop_pool.GetNextLoop();

        // 创建RTMP处理器
        auto handler = new BenchmarkRtmpHandler(i, rtmp_url);

        // 创建RTMP客户端
        auto client = std::make_shared<RtmpClient>(loop, handler);
        clients.push_back(client);

        // 启动客户端（拉流或推流）
        if (config.play_mode)
        {
            client->Play(rtmp_url);
        }
        else
        {
            client->Publish(rtmp_url);
        }

        // 每创建10个客户端，打印进度
        if ((i + 1) % 10 == 0 || (i + 1) == config.client_count)
        {
            std::cout << "已启动 " << (i + 1) << " 个客户端..." << std::endl;
        }

        // 每隔一小段时间创建客户端，避免突发连接
        if (i < config.client_count - 1)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    // 等待测试完成
    int elapsed_seconds = 0;
    while (elapsed_seconds < config.test_duration)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        elapsed_seconds++;

        // 每10秒输出一次临时统计信息
        if (elapsed_seconds % 10 == 0 || elapsed_seconds == config.test_duration)
        {
            std::cout << "已运行 " << elapsed_seconds << " 秒，连接数: " << g_stats.connected_count
                      << std::endl;
        }
    }

    // 测试结束，输出统计信息
    double elapsed = (TTime::NowMS() - start_time) / 1000.0; // 转换为秒
    printStats(config, elapsed);

    // 清理客户端
    std::cout << "测试完成，正在清理资源..." << std::endl;
    clients.clear();

    // 停止线程池前等待任务完成
    std::cout << "等待所有线程完成..." << std::endl;

    // 线程池会在析构时自动清理，无需手动调用Stop

    return 0;
}