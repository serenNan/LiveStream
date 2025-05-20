#!/bin/bash

# RTMP服务器性能监控脚本
# 这个脚本用于收集RTMP服务器的CPU、内存、网络和连接信息

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # 无颜色

# 默认配置
SERVER_IP="127.0.0.1"
SERVER_PORT=1935
INTERVAL=1
DURATION=300
OUTPUT_FILE="server_performance.csv"
SERVER_PID=0
VERBOSE=0

# 打印使用说明
print_usage() {
    echo "用法: $0 [选项]"
    echo "选项:"
    echo "  -s, --server IP       服务器IP地址 (默认: 127.0.0.1)"
    echo "  -p, --port PORT       服务器端口 (默认: 1935)"
    echo "  -i, --interval N      采样间隔(秒) (默认: 1)"
    echo "  -d, --duration N      监控持续时间(秒) (默认: 300, 0表示持续运行)"
    echo "  -o, --output FILE     输出文件 (默认: server_performance.csv)"
    echo "  --pid PID             服务器进程ID (默认: 自动检测)"
    echo "  -v, --verbose         输出详细信息"
    echo "  -h, --help            显示此帮助信息"
}

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -s|--server)
            SERVER_IP="$2"
            shift 2
            ;;
        -p|--port)
            SERVER_PORT="$2"
            shift 2
            ;;
        -i|--interval)
            INTERVAL="$2"
            shift 2
            ;;
        -d|--duration)
            DURATION="$2"
            shift 2
            ;;
        -o|--output)
            OUTPUT_FILE="$2"
            shift 2
            ;;
        --pid)
            SERVER_PID="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        -h|--help)
            print_usage
            exit 0
            ;;
        *)
            echo -e "${RED}未知选项: $1${NC}"
            print_usage
            exit 1
            ;;
    esac
done

# 查找服务器进程ID
find_process_by_port() {
    local port=$1
    pid=$(netstat -tulpn 2>/dev/null | grep "LISTEN" | grep ":$port" | awk '{print $7}' | cut -d/ -f1 | head -n 1)
    echo "$pid"
}

# 如果没有指定PID，尝试自动检测
if [ "$SERVER_PID" -eq 0 ]; then
    SERVER_PID=$(find_process_by_port "$SERVER_PORT")
    if [ -z "$SERVER_PID" ]; then
        echo -e "${YELLOW}警告: 无法自动检测服务器进程ID，将只收集网络和连接统计信息${NC}"
    else
        echo -e "检测到服务器进程ID: ${GREEN}$SERVER_PID${NC}"
    fi
fi

# 获取进程CPU使用率
get_process_cpu_usage() {
    local pid=$1
    if [ -z "$pid" ] || [ "$pid" -eq 0 ]; then
        echo "0.0"
        return
    fi
    
    # 使用ps命令获取CPU使用率
    cpu_usage=$(ps -p "$pid" -o %cpu= 2>/dev/null)
    if [ -z "$cpu_usage" ]; then
        echo "0.0"
    else
        echo "$cpu_usage"
    fi
}

# 获取进程内存使用情况
get_process_memory_usage() {
    local pid=$1
    if [ -z "$pid" ] || [ "$pid" -eq 0 ]; then
        echo "0.0 0.0"
        return
    fi
    
    # 使用ps命令获取内存使用情况
    mem_info=$(ps -p "$pid" -o rss=,%mem= 2>/dev/null)
    if [ -z "$mem_info" ]; then
        echo "0.0 0.0"
    else
        rss=$(echo "$mem_info" | awk '{print $1}')
        mem_percent=$(echo "$mem_info" | awk '{print $2}')
        
        # 将 KB 转换为 MB
        mem_mb=$(echo "scale=2; $rss / 1024" | bc)
        echo "$mem_mb $mem_percent"
    fi
}

# 获取RTMP连接数
get_rtmp_connections() {
    local port=$1
    connections=$(netstat -an | grep ":$port" | grep "ESTABLISHED" | wc -l)
    echo "$connections"
}

# 获取所有网络接口的流量统计
get_all_network_stats() {
    local port=$1
    
    # 使用ss命令获取指定端口的总流量
    if command -v ss &> /dev/null; then
        # 获取发送和接收字节数
        rx_bytes=$(ss -tn "sport = :$port or dport = :$port" 2>/dev/null | awk 'NR>1 {sum_rx += $3} END {print sum_rx}')
        tx_bytes=$(ss -tn "sport = :$port or dport = :$port" 2>/dev/null | awk 'NR>1 {sum_tx += $4} END {print sum_tx}')
        
        # 如果ss命令没有返回结果，使用tcpdump备选
        if [ -z "$rx_bytes" ] || [ -z "$tx_bytes" ]; then
            rx_bytes=0
            tx_bytes=0
            
            # 检查端口流量 - 临时使用tcpdump
            if command -v tcpdump &> /dev/null; then
                # 运行tcpdump 1秒并统计流量
                sample_bytes=$(timeout 1 tcpdump -nn -i any "port $port" -c 50 2>/dev/null | wc -l)
                # 估算流量 - 每个包大约1500字节
                rx_bytes=$((sample_bytes * 1500))
                tx_bytes=$((sample_bytes * 1500))
            fi
        fi
    else
        # 如果没有ss命令，回退到使用/proc/net/dev累加所有接口流量
        rx_bytes=0
        tx_bytes=0
        
        # 获取所有网络接口
        interfaces=$(ls /sys/class/net/)
        
        for interface in $interfaces; do
            if [ -f "/proc/net/dev" ]; then
                stats=$(grep "$interface:" /proc/net/dev 2>/dev/null)
                if [ -n "$stats" ]; then
                    rx=$(echo "$stats" | awk '{print $2}')
                    tx=$(echo "$stats" | awk '{print $10}')
                    rx_bytes=$((rx_bytes + rx))
                    tx_bytes=$((tx_bytes + tx))
                fi
            fi
        done
    fi
    
    # 加入默认的包计数 (简化)
    echo "$rx_bytes 100 $tx_bytes 100"
}

# 获取当前时间戳
get_timestamp() {
    date +%s
}

# 获取当前时间字符串
get_time_string() {
    date +"%Y-%m-%d %H:%M:%S"
}

# 创建CSV文件并写入标题
echo "Timestamp,CPU Usage (%),Memory Usage (MB),Memory Usage (%),RX Bytes,TX Bytes,RX Packets,TX Packets,Active Connections" > "$OUTPUT_FILE"

echo -e "${GREEN}开始监控RTMP服务器性能...${NC}"
echo "服务器: $SERVER_IP:$SERVER_PORT"
echo "采样间隔: $INTERVAL 秒"
if [ "$DURATION" -gt 0 ]; then
    echo "监控持续时间: $DURATION 秒"
else
    echo "监控持续时间: 无限制 (按Ctrl+C停止)"
fi
echo "输出文件: $OUTPUT_FILE"

# 初始化网络统计信息
prev_rx_bytes=0
prev_tx_bytes=0
prev_rx_packets=0
prev_tx_packets=0

# 获取主要网络接口 (现在仅用于显示)
main_interface=$(ip -o -4 route show to default | awk '{print $5}' | head -n 1)
if [ -z "$main_interface" ]; then
    main_interface="eth0" # 默认使用eth0
fi
echo "监控所有网络接口包括环回接口(lo)和 $main_interface"

# 捕获Ctrl+C信号
trap 'echo -e "\n${YELLOW}接收到终止信号，正在退出...${NC}"; exit 0' SIGINT SIGTERM

# 开始时间
start_time=$(date +%s)
elapsed_seconds=0

# 主监控循环
while true; do
    # 当前时间戳
    timestamp=$(get_timestamp)
    
    # 获取CPU使用率
    cpu_usage=$(get_process_cpu_usage "$SERVER_PID")
    
    # 获取内存使用情况
    mem_info=$(get_process_memory_usage "$SERVER_PID")
    mem_mb=$(echo "$mem_info" | awk '{print $1}')
    mem_percent=$(echo "$mem_info" | awk '{print $2}')
    
    # 获取网络统计信息 (包括所有接口)
    net_stats=$(get_all_network_stats "$SERVER_PORT")
    rx_bytes=$(echo "$net_stats" | awk '{print $1}')
    rx_packets=$(echo "$net_stats" | awk '{print $2}')
    tx_bytes=$(echo "$net_stats" | awk '{print $3}')
    tx_packets=$(echo "$net_stats" | awk '{print $4}')
    
    # 获取连接数
    connections=$(get_rtmp_connections "$SERVER_PORT")
    
    # 计算带宽 (更精确的方法)
    rx_bps=0
    tx_bps=0
    if [ "$prev_rx_bytes" -gt 0 ]; then
        # 避免溢出或负值
        if [ "$rx_bytes" -ge "$prev_rx_bytes" ]; then
            rx_bps=$(echo "scale=6; ($rx_bytes - $prev_rx_bytes) * 8 / $INTERVAL" | bc)
        fi
        
        if [ "$tx_bytes" -ge "$prev_tx_bytes" ]; then
            tx_bps=$(echo "scale=6; ($tx_bytes - $prev_tx_bytes) * 8 / $INTERVAL" | bc)
        fi
    fi
    
    prev_rx_bytes=$rx_bytes
    prev_tx_bytes=$tx_bytes
    prev_rx_packets=$rx_packets
    prev_tx_packets=$tx_packets
    
    # 写入CSV
    echo "$timestamp,$cpu_usage,$mem_mb,$mem_percent,$rx_bytes,$tx_bytes,$rx_packets,$tx_packets,$connections" >> "$OUTPUT_FILE"
    
    # 输出信息
    if [ "$VERBOSE" -eq 1 ]; then
        time_str=$(get_time_string)
        # 使用较小的单位展示带宽，如果小于1Mbps，显示Kbps
        if (( $(echo "$rx_bps > 1000000" | bc -l) )); then
            rx_mbps=$(echo "scale=2; $rx_bps / 1000000" | bc)
            rx_display="${rx_mbps} Mbps↓"
        else
            rx_kbps=$(echo "scale=2; $rx_bps / 1000" | bc)
            rx_display="${rx_kbps} Kbps↓"
        fi
        
        if (( $(echo "$tx_bps > 1000000" | bc -l) )); then
            tx_mbps=$(echo "scale=2; $tx_bps / 1000000" | bc)
            tx_display="${tx_mbps} Mbps↑"
        else
            tx_kbps=$(echo "scale=2; $tx_bps / 1000" | bc)
            tx_display="${tx_kbps} Kbps↑"
        fi
        
        echo -e "$time_str - CPU: ${BLUE}$cpu_usage%${NC}, 内存: ${BLUE}$mem_mb MB${NC} (${BLUE}$mem_percent%${NC}), 带宽: ${BLUE}$rx_display${NC} / ${BLUE}$tx_display${NC}, 连接数: ${BLUE}$connections${NC}"
    else
        # 简单进度输出
        echo -ne "监控中... $(get_time_string) - CPU: $cpu_usage%, 连接数: $connections\r"
    fi
    
    # 检查是否达到持续时间
    if [ "$DURATION" -gt 0 ]; then
        elapsed_seconds=$(($(date +%s) - start_time))
        if [ "$elapsed_seconds" -ge "$DURATION" ]; then
            echo -e "\n${GREEN}监控完成，数据已保存到 $OUTPUT_FILE${NC}"
            exit 0
        fi
    fi
    
    # 等待下一个采样间隔
    sleep "$INTERVAL"
done 