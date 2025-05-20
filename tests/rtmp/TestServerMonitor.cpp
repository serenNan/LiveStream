/**
 * @brief RTMP服务器性能监控工具
 *
 * 该程序用于监控RTMP服务器的CPU、内存、网络和连接数等性能指标
 */

#include <arpa/inet.h>
#include <atomic>
#include <chrono>
#include <csignal>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

// 监控配置
struct MonitorConfig
{
    std::string server_ip = "127.0.0.1"; // 服务器IP
    int server_port = 1935;              // 服务器端口
    int interval = 1;                    // 采样间隔(秒)
    int duration = 300;                  // 监控持续时间(秒)，0表示持续运行
    std::string output_file = "server_performance.csv"; // 输出文件
    int server_pid = 0;   // 服务器进程ID，如果为0则自动检测
    bool verbose = false; // 是否输出详细信息
};

// 性能样本结构
struct PerformanceSample
{
    long timestamp;              // 时间戳(秒)
    double cpu_usage;            // CPU使用率(%)
    double memory_usage_mb;      // 内存使用量(MB)
    double memory_usage_percent; // 内存使用率(%)
    long rx_bytes;               // 接收字节数
    long tx_bytes;               // 发送字节数
    long rx_packets;             // 接收包数
    long tx_packets;             // 发送包数
    int active_connections;      // 活动连接数

    // 构造CSV标题行
    static std::string GetCsvHeader()
    {
        return "Timestamp,CPU Usage (%),Memory Usage (MB),Memory Usage (%),RX Bytes,TX Bytes,RX "
               "Packets,TX Packets,Active Connections";
    }

    // 构造CSV数据行
    std::string ToCsvLine() const
    {
        std::stringstream ss;
        ss << timestamp << "," << std::fixed << std::setprecision(2) << cpu_usage << ","
           << std::fixed << std::setprecision(2) << memory_usage_mb << "," << std::fixed
           << std::setprecision(2) << memory_usage_percent << "," << rx_bytes << "," << tx_bytes
           << "," << rx_packets << "," << tx_packets << "," << active_connections;
        return ss.str();
    }
};

// 控制信号标志
std::atomic<bool> g_running{true};

// 信号处理函数
void signalHandler(int signum)
{
    std::cout << "接收到信号 " << signum << ", 准备退出..." << std::endl;
    g_running = false;
}

// 获取当前时间戳(秒)
long getCurrentTimestamp()
{
    return std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

// 获取格式化时间字符串
std::string getTimeString()
{
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// 查找指定端口的进程ID
int findProcessByPort(int port)
{
    // 注意：这种方法依赖于Linux系统
    // 使用netstat获取端口占用进程
    std::string cmd = "netstat -tulpn 2>/dev/null | grep LISTEN | grep :" + std::to_string(port) +
                      " | awk '{print $7}' | cut -d/ -f1";

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        return 0;
    }

    char buffer[128];
    std::string result = "";
    while (!feof(pipe))
    {
        if (fgets(buffer, 128, pipe) != NULL)
        {
            result += buffer;
        }
    }
    pclose(pipe);

    // 移除结尾的换行符
    if (!result.empty() && result[result.length() - 1] == '\n')
    {
        result.erase(result.length() - 1);
    }

    // 尝试将结果转换为整数
    try
    {
        return std::stoi(result);
    }
    catch (...)
    {
        return 0;
    }
}

// 获取进程CPU使用率
double getProcessCpuUsage(int pid)
{
    if (pid <= 0)
    {
        return 0.0;
    }

    // 使用top命令获取CPU使用率
    std::string cmd = "top -b -n 1 -p " + std::to_string(pid) + " | grep " + std::to_string(pid) +
                      " | awk '{print $9}'";

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        return 0.0;
    }

    char buffer[128];
    std::string result = "";
    while (!feof(pipe))
    {
        if (fgets(buffer, 128, pipe) != NULL)
        {
            result += buffer;
        }
    }
    pclose(pipe);

    // 移除结尾的换行符
    if (!result.empty() && result[result.length() - 1] == '\n')
    {
        result.erase(result.length() - 1);
    }

    // 尝试将结果转换为浮点数
    try
    {
        return std::stod(result);
    }
    catch (...)
    {
        return 0.0;
    }
}

// 获取进程内存使用情况
void getProcessMemoryUsage(int pid, double &memory_mb, double &memory_percent)
{
    memory_mb = 0.0;
    memory_percent = 0.0;

    if (pid <= 0)
    {
        return;
    }

    // 使用ps命令获取内存使用情况
    std::string cmd = "ps -p " + std::to_string(pid) + " -o rss,%mem | grep -v RSS";

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        return;
    }

    char buffer[128];
    std::string result = "";
    while (!feof(pipe))
    {
        if (fgets(buffer, 128, pipe) != NULL)
        {
            result += buffer;
        }
    }
    pclose(pipe);

    // 解析结果
    try
    {
        std::istringstream iss(result);
        long rss;
        iss >> rss >> memory_percent;
        memory_mb = rss / 1024.0; // 转换KB到MB
    }
    catch (...)
    {
        memory_mb = 0.0;
        memory_percent = 0.0;
    }
}

// 获取网络接口统计信息
void getNetworkStats(const std::string &interface, long &rx_bytes, long &tx_bytes, long &rx_packets,
                     long &tx_packets)
{
    rx_bytes = 0;
    tx_bytes = 0;
    rx_packets = 0;
    tx_packets = 0;

    std::ifstream net_dev("/proc/net/dev");
    if (!net_dev.is_open())
    {
        return;
    }

    std::string line;
    while (std::getline(net_dev, line))
    {
        if (line.find(interface + ":") != std::string::npos)
        {
            std::istringstream iss(line);
            std::string skip;
            iss >> skip; // 跳过接口名称
            iss >> rx_bytes >> rx_packets;

            // 跳过6个字段
            for (int i = 0; i < 6; ++i)
            {
                iss >> skip;
            }

            iss >> tx_bytes >> tx_packets;
            break;
        }
    }

    net_dev.close();
}

// 获取RTMP连接数
int getRtmpConnections(const std::string &server_ip, int server_port)
{
    // 这个方法无法精确获取RTMP连接数，您可能需要服务器提供API来获取
    // 这里使用一个简单的方法：统计与特定端口的连接数
    std::string cmd =
        "netstat -an | grep " + std::to_string(server_port) + " | grep ESTABLISHED | wc -l";

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        return 0;
    }

    char buffer[128];
    std::string result = "";
    while (!feof(pipe))
    {
        if (fgets(buffer, 128, pipe) != NULL)
        {
            result += buffer;
        }
    }
    pclose(pipe);

    // 尝试将结果转换为整数
    try
    {
        return std::stoi(result);
    }
    catch (...)
    {
        return 0;
    }
}

// 收集性能样本
PerformanceSample collectPerformanceSample(const MonitorConfig &config, int pid)
{
    PerformanceSample sample;
    sample.timestamp = getCurrentTimestamp();

    // 收集CPU使用率
    sample.cpu_usage = getProcessCpuUsage(pid);

    // 收集内存使用情况
    getProcessMemoryUsage(pid, sample.memory_usage_mb, sample.memory_usage_percent);

    // 收集网络统计信息 (使用eth0作为默认接口，您可能需要修改为实际接口)
    getNetworkStats("eth0", sample.rx_bytes, sample.tx_bytes, sample.rx_packets, sample.tx_packets);

    // 收集连接数
    sample.active_connections = getRtmpConnections(config.server_ip, config.server_port);

    return sample;
}

// 打印使用说明
void printUsage(const char *program)
{
    std::cout << "用法: " << program << " [选项]" << std::endl;
    std::cout << "选项:" << std::endl;
    std::cout << "  --server IP       服务器IP地址 (默认: 127.0.0.1)" << std::endl;
    std::cout << "  --port PORT       服务器端口 (默认: 1935)" << std::endl;
    std::cout << "  --interval N      采样间隔(秒) (默认: 1)" << std::endl;
    std::cout << "  --duration N      监控持续时间(秒) (默认: 300, 0表示持续运行)" << std::endl;
    std::cout << "  --output FILE     输出文件 (默认: server_performance.csv)" << std::endl;
    std::cout << "  --pid PID         服务器进程ID (默认: 自动检测)" << std::endl;
    std::cout << "  --verbose         输出详细信息" << std::endl;
    std::cout << "  --help            显示此帮助信息" << std::endl;
}

// 解析命令行参数
bool parseArgs(int argc, const char **argv, MonitorConfig &config)
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
        else if (arg == "--interval" && i + 1 < argc)
        {
            config.interval = std::stoi(argv[++i]);
        }
        else if (arg == "--duration" && i + 1 < argc)
        {
            config.duration = std::stoi(argv[++i]);
        }
        else if (arg == "--output" && i + 1 < argc)
        {
            config.output_file = argv[++i];
        }
        else if (arg == "--pid" && i + 1 < argc)
        {
            config.server_pid = std::stoi(argv[++i]);
        }
        else if (arg == "--verbose")
        {
            config.verbose = true;
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

    // 验证配置
    if (config.interval < 1)
    {
        std::cerr << "采样间隔必须大于等于1秒" << std::endl;
        return false;
    }

    return true;
}

int main(int argc, const char **argv)
{
    // 注册信号处理函数
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // 配置参数
    MonitorConfig config;

    // 解析命令行参数
    if (!parseArgs(argc, argv, config))
    {
        printUsage(argv[0]);
        return 1;
    }

    // 如果没有指定PID，尝试自动检测
    if (config.server_pid == 0)
    {
        config.server_pid = findProcessByPort(config.server_port);
        if (config.server_pid == 0)
        {
            std::cerr << "警告: 无法自动检测服务器进程ID，将只收集网络和连接统计信息" << std::endl;
        }
        else
        {
            std::cout << "检测到服务器进程ID: " << config.server_pid << std::endl;
        }
    }

    std::cout << "开始监控RTMP服务器性能..." << std::endl;
    std::cout << "服务器: " << config.server_ip << ":" << config.server_port << std::endl;
    std::cout << "采样间隔: " << config.interval << " 秒" << std::endl;
    if (config.duration > 0)
    {
        std::cout << "监控持续时间: " << config.duration << " 秒" << std::endl;
    }
    else
    {
        std::cout << "监控持续时间: 无限制 (按Ctrl+C停止)" << std::endl;
    }
    std::cout << "输出文件: " << config.output_file << std::endl;

    // 创建输出文件
    std::ofstream output_file(config.output_file);
    if (!output_file.is_open())
    {
        std::cerr << "错误: 无法创建输出文件 " << config.output_file << std::endl;
        return 1;
    }

    // 写入CSV标题
    output_file << PerformanceSample::GetCsvHeader() << std::endl;

    // 开始时间
    auto start_time = std::chrono::high_resolution_clock::now();
    int elapsed_seconds = 0;

    // 前一次的网络统计数据
    long prev_rx_bytes = 0, prev_tx_bytes = 0;
    long prev_rx_packets = 0, prev_tx_packets = 0;

    // 主监控循环
    while (g_running)
    {
        // 收集样本
        PerformanceSample sample = collectPerformanceSample(config, config.server_pid);

        // 计算带宽
        long rx_bps = 0, tx_bps = 0;
        if (prev_rx_bytes > 0)
        {
            rx_bps = (sample.rx_bytes - prev_rx_bytes) * 8 / config.interval; // bits per second
            tx_bps = (sample.tx_bytes - prev_tx_bytes) * 8 / config.interval; // bits per second
        }

        prev_rx_bytes = sample.rx_bytes;
        prev_tx_bytes = sample.tx_bytes;
        prev_rx_packets = sample.rx_packets;
        prev_tx_packets = sample.tx_packets;

        // 写入CSV
        output_file << sample.ToCsvLine() << std::endl;

        // 如果verbose模式，输出详细信息
        if (config.verbose)
        {
            std::cout << getTimeString() << " - ";
            std::cout << "CPU: " << std::fixed << std::setprecision(2) << sample.cpu_usage << "%, ";
            std::cout << "内存: " << std::fixed << std::setprecision(2) << sample.memory_usage_mb
                      << "MB (";
            std::cout << std::fixed << std::setprecision(2) << sample.memory_usage_percent
                      << "%), ";
            std::cout << "带宽: " << (rx_bps / 1000000.0) << "Mbps↓ / " << (tx_bps / 1000000.0)
                      << "Mbps↑, ";
            std::cout << "连接数: " << sample.active_connections << std::endl;
        }
        else
        {
            // 简单进度输出
            std::cout << "监控中... " << getTimeString() << " - ";
            std::cout << "CPU: " << std::fixed << std::setprecision(2) << sample.cpu_usage << "%, ";
            std::cout << "连接数: " << sample.active_connections << "\r" << std::flush;
        }

        // 检查是否达到持续时间
        if (config.duration > 0)
        {
            elapsed_seconds += config.interval;
            if (elapsed_seconds >= config.duration)
            {
                g_running = false;
            }
        }

        // 等待下一个采样间隔
        std::this_thread::sleep_for(std::chrono::seconds(config.interval));
    }

    std::cout << std::endl << "监控完成，数据已保存到 " << config.output_file << std::endl;

    // 关闭输出文件
    output_file.close();

    return 0;
}
*@brief RTMP服务器性能监控工具 **该程序用于监控RTMP服务器的CPU、内存、网络和连接数等性能指标 * /

#include <arpa/inet.h>
#include <atomic>
#include <chrono>
#include <csignal>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

    // 监控配置
    struct MonitorConfig
{
    std::string server_ip = "127.0.0.1"; // 服务器IP
    int server_port = 1935;              // 服务器端口
    int interval = 1;                    // 采样间隔(秒)
    int duration = 300;                  // 监控持续时间(秒)，0表示持续运行
    std::string output_file = "server_performance.csv"; // 输出文件
    int server_pid = 0;   // 服务器进程ID，如果为0则自动检测
    bool verbose = false; // 是否输出详细信息
};

// 性能样本结构
struct PerformanceSample
{
    long timestamp;              // 时间戳(秒)
    double cpu_usage;            // CPU使用率(%)
    double memory_usage_mb;      // 内存使用量(MB)
    double memory_usage_percent; // 内存使用率(%)
    long rx_bytes;               // 接收字节数
    long tx_bytes;               // 发送字节数
    long rx_packets;             // 接收包数
    long tx_packets;             // 发送包数
    int active_connections;      // 活动连接数

    // 构造CSV标题行
    static std::string GetCsvHeader()
    {
        return "Timestamp,CPU Usage (%),Memory Usage (MB),Memory Usage (%),RX Bytes,TX Bytes,RX "
               "Packets,TX Packets,Active Connections";
    }

    // 构造CSV数据行
    std::string ToCsvLine() const
    {
        std::stringstream ss;
        ss << timestamp << "," << std::fixed << std::setprecision(2) << cpu_usage << ","
           << std::fixed << std::setprecision(2) << memory_usage_mb << "," << std::fixed
           << std::setprecision(2) << memory_usage_percent << "," << rx_bytes << "," << tx_bytes
           << "," << rx_packets << "," << tx_packets << "," << active_connections;
        return ss.str();
    }
};

// 控制信号标志
std::atomic<bool> g_running{true};

// 信号处理函数
void signalHandler(int signum)
{
    std::cout << "接收到信号 " << signum << ", 准备退出..." << std::endl;
    g_running = false;
}
