#!/bin/bash

# RTMP流量监控脚本
# 此脚本专门用于监控RTMP端口流量，使用tcpdump直接捕获数据包

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # 无颜色

# 默认配置
PORT=1935
DURATION=60
INTERFACE="any"
VERBOSE=1
OUTPUT_FILE="rtmp_traffic.log"

# 使用说明
print_usage() {
    echo "用法: $0 [选项]"
    echo "选项:"
    echo "  -p, --port PORT       RTMP端口 (默认: 1935)"
    echo "  -i, --interface IF    网络接口 (默认: any，监控所有接口)"
    echo "  -d, --duration SEC    持续时间(秒) (默认: 60)"
    echo "  -o, --output FILE     输出文件 (默认: rtmp_traffic.log)"
    echo "  -h, --help            显示帮助信息"
}

# 参数解析
while [[ $# -gt 0 ]]; do
    case $1 in
        -p|--port)
            PORT="$2"
            shift 2
            ;;
        -i|--interface)
            INTERFACE="$2"
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

# 检查tcpdump是否安装
if ! command -v tcpdump &> /dev/null; then
    echo -e "${RED}错误: 未安装tcpdump。请执行 'sudo apt install tcpdump' 安装${NC}"
    exit 1
fi

# 确认是否有必要的权限
if [ "$(id -u)" -ne 0 ]; then
    echo -e "${YELLOW}警告: 没有root权限，tcpdump可能无法捕获所有数据包。${NC}"
    echo -e "${YELLOW}建议使用 'sudo $0 ${@}' 重新运行脚本。${NC}"
    echo -e "${YELLOW}尝试继续运行...${NC}"
    echo ""
fi

# 初始化计数器
total_packets=0
data_bytes_in=0
data_bytes_out=0
last_check_time=$(date +%s)
start_time=$last_check_time

echo -e "${GREEN}开始监控RTMP流量...${NC}"
echo "端口: $PORT"
echo "接口: $INTERFACE"
echo "持续时间: $DURATION 秒"
echo "输出文件: $OUTPUT_FILE"
echo ""

# 启动新日志文件
echo "时间戳,Kbps接收,Kbps发送,总包数,包/秒" > "$OUTPUT_FILE"

# 捕获Ctrl+C信号
trap 'echo -e "\n${YELLOW}接收到终止信号，正在退出...${NC}"; exit 0' SIGINT SIGTERM

# 启动tcpdump并实时分析
tcpdump -i "$INTERFACE" -n "port $PORT" -l 2>/dev/null | while read -r line; do
    current_time=$(date +%s)
    elapsed=$((current_time - start_time))
    
    # 如果超过持续时间，退出
    if [ "$DURATION" -gt 0 ] && [ "$elapsed" -ge "$DURATION" ]; then
        echo -e "\n${GREEN}监控完成，数据已保存到 $OUTPUT_FILE${NC}"
        exit 0
    fi
    
    # 更新计数器
    ((total_packets++))
    
    # 提取数据包大小 (近似值)
    if [[ $line =~ length\ ([0-9]+) ]]; then
        packet_size="${BASH_REMATCH[1]}"
        
        # 判断数据流方向
        if [[ $line =~ \>\ [0-9.]+\.$PORT ]]; then
            # 传入RTMP服务器的数据
            ((data_bytes_in += packet_size))
        else
            # 从RTMP服务器传出的数据
            ((data_bytes_out += packet_size))
        fi
    fi
    
    # 每秒更新一次统计
    if ((current_time > last_check_time)); then
        interval=$((current_time - last_check_time))
        last_check_time=$current_time
        
        # 计算当前带宽 (Kbps)
        kbps_in=$(echo "scale=2; ($data_bytes_in * 8) / ($interval * 1000)" | bc)
        kbps_out=$(echo "scale=2; ($data_bytes_out * 8) / ($interval * 1000)" | bc)
        pps=$((total_packets / interval))
        
        # 输出当前带宽
        time_str=$(date +"%Y-%m-%d %H:%M:%S")
        echo -e "$time_str - 入站流量: ${BLUE}$kbps_in Kbps↓${NC}, 出站流量: ${BLUE}$kbps_out Kbps↑${NC}, 包速率: ${BLUE}$pps 包/秒${NC}"
        
        # 写入CSV
        echo "$current_time,$kbps_in,$kbps_out,$total_packets,$pps" >> "$OUTPUT_FILE"
        
        # 重置单位时间内的计数
        data_bytes_in=0
        data_bytes_out=0
        total_packets=0
    fi
done

echo -e "\n${GREEN}监控完成，数据已保存到 $OUTPUT_FILE${NC}" 