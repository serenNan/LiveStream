#!/bin/bash

# 直播系统性能测试脚本
# 此脚本用于自动化运行RTMP直播系统的性能测试

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # 无颜色

# 结果目录
RESULTS_DIR="performance_results"

# 检查build目录
if [ ! -d "../build" ]; then
    echo -e "${RED}错误: build目录不存在，请先编译项目${NC}"
    exit 1
fi

# 进入build目录
cd ../build || exit 1

# 检查是否已编译测试程序
if [ ! -f "tests/TestRtmpBenchmark" ] || [ ! -f "tests/TestRtmpLatency" ]; then
    echo -e "${YELLOW}未找到测试程序，尝试编译...${NC}"
    make TestRtmpBenchmark TestRtmpLatency
fi

# 再次检查测试程序是否存在
if [ ! -f "tests/TestRtmpBenchmark" ] || [ ! -f "tests/TestRtmpLatency" ]; then
    echo -e "${RED}错误: 测试程序编译失败${NC}"
    exit 1
fi

# 检查Python可视化依赖
check_python_deps() {
    python3 -c "import pandas" 2>/dev/null
    if [ $? -ne 0 ]; then
        echo -e "${YELLOW}缺少pandas库，尝试安装...${NC}"
        pip3 install pandas
    fi
    
    python3 -c "import matplotlib" 2>/dev/null
    if [ $? -ne 0 ]; then
        echo -e "${YELLOW}缺少matplotlib库，尝试安装...${NC}"
        pip3 install matplotlib
    fi
}

# 创建结果目录
create_results_dir() {
    local timestamp=$(date +"%Y%m%d_%H%M%S")
    local result_dir="$RESULTS_DIR/$timestamp"
    
    mkdir -p "$result_dir"
    echo "$result_dir"
}

# 定义测试函数
run_benchmark_test() {
    local clients=$1
    local duration=$2
    local mode=$3
    local server_ip=$4
    local port=$5
    local stream=$6
    local result_dir=$7
    
    echo -e "${BLUE}=== 运行RTMP并发测试 ===${NC}"
    echo -e "${BLUE}客户端数量: $clients ${NC}"
    echo -e "${BLUE}测试持续时间: $duration 秒${NC}"
    echo -e "${BLUE}测试模式: $mode ${NC}"
    
    # 创建输出文件名
    local output_file="$result_dir/${mode}_${clients}_clients.log"
    
    # 启动服务器监控
    ./server_monitor.sh -s "$server_ip" -p "$port" -d "$duration" -o "$result_dir/server_perf_${mode}_${clients}.csv" -v &
    local monitor_pid=$!
    
    # 启动RTMP流量监控 (使用sudo确保能捕获所有数据包)
    echo -e "${YELLOW}请输入密码以启动RTMP流量监控...${NC}"
    sudo ./rtmp_traffic_monitor.sh -p "$port" -d "$duration" -o "$result_dir/rtmp_traffic_${mode}_${clients}.log" &
    local traffic_monitor_pid=$!
    
    # 运行测试
    if [ "$mode" == "play" ]; then
        ./tests/TestRtmpBenchmark --clients "$clients" --duration "$duration" --server "$server_ip" --port "$port" --stream "$stream" --play > "$output_file" 2>&1
    else
        ./tests/TestRtmpBenchmark --clients "$clients" --duration "$duration" --server "$server_ip" --port "$port" --stream "$stream" --publish > "$output_file" 2>&1
    fi
    
    # 等待监控进程完成
    wait $monitor_pid
    wait $traffic_monitor_pid
    
    # 分析结果
    python3 ../tests/visualize_perf_data.py "$result_dir/server_perf_${mode}_${clients}.csv" --output "$result_dir/report_${mode}_${clients}.pdf" --title "RTMP ${mode}模式 ${clients}客户端 性能报告" --no-show
    
    echo -e "${GREEN}测试完成，结果已保存到 $output_file${NC}"
    echo -e "${GREEN}性能报告已保存到 $result_dir/report_${mode}_${clients}.pdf${NC}"
    echo -e "${GREEN}流量监控数据已保存到 $result_dir/rtmp_traffic_${mode}_${clients}.log${NC}"
}

run_latency_test() {
    local duration=$1
    local interval=$2
    local server_ip=$3
    local port=$4
    local stream=$5
    local result_dir=$6

    echo -e "${BLUE}=== 运行RTMP延迟测试 ===${NC}"
    echo -e "${BLUE}测试持续时间: $duration 秒${NC}"
    echo -e "${BLUE}采样间隔: $interval 毫秒${NC}"
    
    # 创建输出文件名
    local output_file="$result_dir/latency_test.log"
    
    # 启动服务器监控
    ./server_monitor.sh -s "$server_ip" -p "$port" -d "$duration" -o "$result_dir/server_perf_latency.csv" -v &
    local monitor_pid=$!
    
    # 启动RTMP流量监控 (使用sudo确保能捕获所有数据包)
    echo -e "${YELLOW}请输入密码以启动RTMP流量监控...${NC}"
    sudo ./rtmp_traffic_monitor.sh -p "$port" -d "$duration" -o "$result_dir/rtmp_traffic_latency.log" &
    local traffic_monitor_pid=$!
    
    # 运行测试
    ./tests/TestRtmpLatency --duration "$duration" --interval "$interval" --server "$server_ip" --port "$port" --stream "$stream" > "$output_file" 2>&1
    
    # 等待监控进程完成
    wait $monitor_pid
    wait $traffic_monitor_pid
    
    # 分析结果
    python3 ../tests/visualize_perf_data.py "$result_dir/server_perf_latency.csv" --output "$result_dir/report_latency.pdf" --title "RTMP 延迟测试 性能报告" --no-show
    
    echo -e "${GREEN}测试完成，结果已保存到 $output_file${NC}"
    echo -e "${GREEN}性能报告已保存到 $result_dir/report_latency.pdf${NC}"
    echo -e "${GREEN}流量监控数据已保存到 $result_dir/rtmp_traffic_latency.log${NC}"
}

# 检查服务器监控脚本
check_server_monitor() {
    if [ ! -f "./server_monitor.sh" ]; then
        echo -e "${YELLOW}未找到服务器监控脚本，正在复制...${NC}"
        cp ../tests/server_monitor.sh .
        chmod +x ./server_monitor.sh
    else
        # 更新现有的监控脚本，确保使用最新版本
        cp ../tests/server_monitor.sh .
        chmod +x ./server_monitor.sh
        echo -e "${GREEN}已更新服务器监控脚本${NC}"
    fi
    
    # 检查流量监控脚本
    if [ ! -f "./rtmp_traffic_monitor.sh" ]; then
        echo -e "${YELLOW}未找到RTMP流量监控脚本，正在复制...${NC}"
        cp ../tests/rtmp_traffic_monitor.sh .
        chmod +x ./rtmp_traffic_monitor.sh
    else
        # 更新现有的监控脚本，确保使用最新版本
        cp ../tests/rtmp_traffic_monitor.sh .
        chmod +x ./rtmp_traffic_monitor.sh
        echo -e "${GREEN}已更新RTMP流量监控脚本${NC}"
    fi
}

# 显示菜单
show_menu() {
    echo -e "${GREEN}===== 直播系统性能测试 =====${NC}"
    echo "1) 并发拉流测试 (100客户端)"
    echo "2) 并发拉流测试 (500客户端)"
    echo "3) 并发推流测试 (10客户端)"
    echo "4) 端到端延迟测试"
    echo "5) 全套性能测试"
    echo "6) 查看性能报告"
    echo "0) 退出"
    echo -n "请选择测试类型 [0-6]: "
}

# 获取服务器配置
get_server_config() {
    echo -n "输入服务器IP地址 [默认: 127.0.0.1]: "
    read -r server_ip
    server_ip=${server_ip:-127.0.0.1}
    
    echo -n "输入服务器端口 [默认: 1935]: "
    read -r port
    port=${port:-1935}
    
    echo -n "输入流名称 [默认: test]: "
    read -r stream
    stream=${stream:-test}
}

# 打开最新的报告
open_latest_report() {
    local latest_dir=$(find "$RESULTS_DIR" -type d | sort | tail -n 1)
    
    if [ -z "$latest_dir" ]; then
        echo -e "${RED}未找到任何测试报告${NC}"
        return
    fi
    
    echo -e "${BLUE}最新测试报告目录: $latest_dir${NC}"
    echo "可用报告:"
    
    local reports=($(find "$latest_dir" -name "*.pdf"))
    
    if [ ${#reports[@]} -eq 0 ]; then
        echo -e "${RED}未找到任何PDF报告${NC}"
        return
    fi
    
    for i in "${!reports[@]}"; do
        echo "$((i+1))) ${reports[$i]##*/}"
    done
    
    echo -n "请选择要查看的报告 [1-${#reports[@]}]: "
    read -r choice
    
    if [[ "$choice" -ge 1 && "$choice" -le "${#reports[@]}" ]]; then
        local report_file="${reports[$((choice-1))]}"
        echo -e "${GREEN}打开报告: $report_file${NC}"
        
        # 尝试使用不同的PDF查看器
        if command -v xdg-open > /dev/null; then
            xdg-open "$report_file"
        elif command -v evince > /dev/null; then
            evince "$report_file"
        elif command -v okular > /dev/null; then
            okular "$report_file"
        else
            echo -e "${YELLOW}未找到PDF查看器，请手动打开文件: $report_file${NC}"
        fi
    else
        echo -e "${RED}无效选择${NC}"
    fi
}

# 主函数
main() {
    # 检查Python依赖
    check_python_deps
    
    # 检查服务器监控脚本
    check_server_monitor
    
    # 创建结果目录
    mkdir -p "$RESULTS_DIR"
    result_dir=$(create_results_dir)
    echo -e "${GREEN}测试结果将保存到: $result_dir${NC}"
    
    # 获取服务器配置
    get_server_config
    
    while true; do
        show_menu
        read -r choice
        
        case $choice in
            1)
                run_benchmark_test 100 60 "play" "$server_ip" "$port" "$stream" "$result_dir"
                ;;
            2)
                run_benchmark_test 500 60 "play" "$server_ip" "$port" "$stream" "$result_dir"
                ;;
            3)
                run_benchmark_test 10 60 "publish" "$server_ip" "$port" "$stream" "$result_dir"
                ;;
            4)
                run_latency_test 60 1000 "$server_ip" "$port" "latency_test" "$result_dir"
                ;;
            5)
                echo -e "${YELLOW}运行全套性能测试...${NC}"
                run_benchmark_test 100 30 "play" "$server_ip" "$port" "$stream" "$result_dir"
                sleep 5
                run_benchmark_test 10 30 "publish" "$server_ip" "$port" "$stream" "$result_dir"
                sleep 5
                run_latency_test 30 1000 "$server_ip" "$port" "latency_test" "$result_dir"
                ;;
            6)
                open_latest_report
                ;;
            0)
                echo -e "${YELLOW}退出测试...${NC}"
                exit 0
                ;;
            *)
                echo -e "${RED}无效选择，请重试${NC}"
                ;;
        esac
    done
}

# 启动主函数
main 