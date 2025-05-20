#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
RTMP直播系统性能测试数据可视化工具
该脚本用于分析和可视化性能测试的结果数据
"""

import os
import sys
import argparse
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from datetime import datetime

def parse_arguments():
    """解析命令行参数"""
    parser = argparse.ArgumentParser(description='RTMP性能测试数据可视化工具')
    parser.add_argument('file', type=str, help='CSV数据文件路径')
    parser.add_argument('--output', '-o', type=str, default='performance_report.pdf',
                      help='输出报告文件名 (默认: performance_report.pdf)')
    parser.add_argument('--title', '-t', type=str, default='RTMP服务器性能报告',
                      help='报告标题')
    parser.add_argument('--no-show', action='store_true',
                      help='不显示图形界面，只保存文件')
    return parser.parse_args()

def load_data(filename):
    """加载CSV数据文件"""
    try:
        df = pd.read_csv(filename)
        # 将时间戳转换为datetime
        df['Datetime'] = pd.to_datetime(df['Timestamp'], unit='s')
        # 计算从开始的时间(秒)
        df['Elapsed'] = df['Timestamp'] - df['Timestamp'].iloc[0]
        return df
    except Exception as e:
        print(f"错误: 无法加载数据文件 '{filename}': {e}")
        sys.exit(1)

def calculate_metrics(df):
    """计算关键性能指标"""
    metrics = {}
    
    # CPU使用率
    metrics['cpu_mean'] = df['CPU Usage (%)'].mean()
    metrics['cpu_max'] = df['CPU Usage (%)'].max()
    
    # 内存使用率
    metrics['mem_mean'] = df['Memory Usage (MB)'].mean()
    metrics['mem_max'] = df['Memory Usage (MB)'].max()
    
    # 带宽计算
    if len(df) > 1:
        # 计算带宽 (bits per second)
        df['RX Bandwidth'] = df['RX Bytes'].diff() * 8 / df['Timestamp'].diff()
        df['TX Bandwidth'] = df['TX Bytes'].diff() * 8 / df['Timestamp'].diff()
        
        # 转换为Mbps
        df['RX Mbps'] = df['RX Bandwidth'] / 1000000
        df['TX Mbps'] = df['TX Bandwidth'] / 1000000
        
        # 删除第一行(NaN值)
        df_bandwidth = df.dropna(subset=['RX Bandwidth', 'TX Bandwidth'])
        
        metrics['rx_mbps_mean'] = df_bandwidth['RX Mbps'].mean()
        metrics['rx_mbps_max'] = df_bandwidth['RX Mbps'].max()
        metrics['tx_mbps_mean'] = df_bandwidth['TX Mbps'].mean()
        metrics['tx_mbps_max'] = df_bandwidth['TX Mbps'].max()
    else:
        metrics['rx_mbps_mean'] = 0
        metrics['rx_mbps_max'] = 0
        metrics['tx_mbps_mean'] = 0
        metrics['tx_mbps_max'] = 0
    
    # 连接数
    metrics['conn_mean'] = df['Active Connections'].mean()
    metrics['conn_max'] = df['Active Connections'].max()
    
    # 时间范围
    metrics['start_time'] = df['Datetime'].iloc[0]
    metrics['end_time'] = df['Datetime'].iloc[-1]
    metrics['duration'] = (metrics['end_time'] - metrics['start_time']).total_seconds()
    
    return metrics

def create_plots(df, metrics, title):
    """创建性能图表"""
    # 设置图表样式
    plt.style.use('ggplot')
    plt.rcParams['font.sans-serif'] = ['SimHei']  # 用来正常显示中文标签
    plt.rcParams['axes.unicode_minus'] = False  # 用来正常显示负号
    
    # 创建4个子图
    fig, axes = plt.subplots(4, 1, figsize=(12, 16))
    fig.suptitle(title, fontsize=16)
    
    # 时间轴
    x = df['Elapsed']
    
    # 1. CPU使用率
    axes[0].plot(x, df['CPU Usage (%)'], 'b-', linewidth=1)
    axes[0].set_title('CPU使用率')
    axes[0].set_ylabel('CPU使用率 (%)')
    axes[0].set_xlabel('时间 (秒)')
    axes[0].grid(True)
    
    # 添加平均值线
    axes[0].axhline(y=metrics['cpu_mean'], color='r', linestyle='--', 
                   label=f'平均: {metrics["cpu_mean"]:.2f}%')
    axes[0].legend()
    
    # 2. 内存使用
    axes[1].plot(x, df['Memory Usage (MB)'], 'g-', linewidth=1)
    axes[1].set_title('内存使用')
    axes[1].set_ylabel('内存 (MB)')
    axes[1].set_xlabel('时间 (秒)')
    axes[1].grid(True)
    
    # 添加平均值线
    axes[1].axhline(y=metrics['mem_mean'], color='r', linestyle='--', 
                   label=f'平均: {metrics["mem_mean"]:.2f} MB')
    axes[1].legend()
    
    # 3. 网络带宽
    if 'RX Mbps' in df.columns and 'TX Mbps' in df.columns:
        df_bandwidth = df.dropna(subset=['RX Mbps', 'TX Mbps'])
        x_bw = df_bandwidth['Elapsed']
        
        axes[2].plot(x_bw, df_bandwidth['RX Mbps'], 'b-', linewidth=1, label='接收')
        axes[2].plot(x_bw, df_bandwidth['TX Mbps'], 'g-', linewidth=1, label='发送')
        axes[2].set_title('网络带宽')
        axes[2].set_ylabel('带宽 (Mbps)')
        axes[2].set_xlabel('时间 (秒)')
        axes[2].grid(True)
        
        # 添加平均值线
        axes[2].axhline(y=metrics['rx_mbps_mean'], color='b', linestyle='--', 
                       label=f'平均接收: {metrics["rx_mbps_mean"]:.2f} Mbps')
        axes[2].axhline(y=metrics['tx_mbps_mean'], color='g', linestyle='--', 
                       label=f'平均发送: {metrics["tx_mbps_mean"]:.2f} Mbps')
        axes[2].legend()
    
    # 4. 活动连接数
    axes[3].plot(x, df['Active Connections'], 'r-', linewidth=1)
    axes[3].set_title('活动连接数')
    axes[3].set_ylabel('连接数')
    axes[3].set_xlabel('时间 (秒)')
    axes[3].grid(True)
    
    # 添加平均值线
    axes[3].axhline(y=metrics['conn_mean'], color='b', linestyle='--', 
                   label=f'平均: {metrics["conn_mean"]:.1f}')
    axes[3].legend()
    
    plt.tight_layout(rect=[0, 0, 1, 0.97])
    return fig

def create_summary_table(metrics):
    """创建性能摘要表格"""
    fig, ax = plt.subplots(figsize=(8, 6))
    fig.suptitle('性能测试摘要', fontsize=14)
    
    # 隐藏坐标轴
    ax.axis('tight')
    ax.axis('off')
    
    # 表格数据
    data = [
        ['开始时间', metrics['start_time'].strftime('%Y-%m-%d %H:%M:%S')],
        ['结束时间', metrics['end_time'].strftime('%Y-%m-%d %H:%M:%S')],
        ['测试持续时间', f"{metrics['duration']:.2f} 秒"],
        ['平均 CPU 使用率', f"{metrics['cpu_mean']:.2f}%"],
        ['最大 CPU 使用率', f"{metrics['cpu_max']:.2f}%"],
        ['平均内存使用', f"{metrics['mem_mean']:.2f} MB"],
        ['最大内存使用', f"{metrics['mem_max']:.2f} MB"],
        ['平均接收带宽', f"{metrics['rx_mbps_mean']:.2f} Mbps"],
        ['最大接收带宽', f"{metrics['rx_mbps_max']:.2f} Mbps"],
        ['平均发送带宽', f"{metrics['tx_mbps_mean']:.2f} Mbps"],
        ['最大发送带宽', f"{metrics['tx_mbps_max']:.2f} Mbps"],
        ['平均活动连接数', f"{metrics['conn_mean']:.1f}"],
        ['最大活动连接数', f"{metrics['conn_max']:.0f}"]
    ]
    
    # 创建表格
    table = ax.table(cellText=data, colWidths=[0.3, 0.5], loc='center')
    table.auto_set_font_size(False)
    table.set_fontsize(10)
    table.scale(1, 1.5)
    
    plt.tight_layout()
    return fig

def print_summary(metrics):
    """打印性能摘要到控制台"""
    print("\n========== RTMP服务器性能测试摘要 ==========")
    print(f"测试时间: {metrics['start_time'].strftime('%Y-%m-%d %H:%M:%S')} - {metrics['end_time'].strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"测试持续时间: {metrics['duration']:.2f} 秒")
    print("\nCPU使用率:")
    print(f"  平均: {metrics['cpu_mean']:.2f}%")
    print(f"  最大: {metrics['cpu_max']:.2f}%")
    print("\n内存使用:")
    print(f"  平均: {metrics['mem_mean']:.2f} MB")
    print(f"  最大: {metrics['mem_max']:.2f} MB")
    print("\n网络带宽:")
    print(f"  平均接收: {metrics['rx_mbps_mean']:.2f} Mbps")
    print(f"  最大接收: {metrics['rx_mbps_max']:.2f} Mbps")
    print(f"  平均发送: {metrics['tx_mbps_mean']:.2f} Mbps")
    print(f"  最大发送: {metrics['tx_mbps_max']:.2f} Mbps")
    print("\n连接数:")
    print(f"  平均: {metrics['conn_mean']:.1f}")
    print(f"  最大: {metrics['conn_max']:.0f}")
    print("==============================================")

def main():
    """主函数"""
    args = parse_arguments()
    
    # 加载数据
    print(f"加载数据文件: {args.file}")
    df = load_data(args.file)
    
    # 计算指标
    print("计算性能指标...")
    metrics = calculate_metrics(df)
    
    # 打印摘要
    print_summary(metrics)
    
    # 创建图表
    print("创建性能图表...")
    perf_fig = create_plots(df, metrics, args.title)
    summary_fig = create_summary_table(metrics)
    
    # 保存报告
    from matplotlib.backends.backend_pdf import PdfPages
    
    print(f"保存报告到: {args.output}")
    with PdfPages(args.output) as pdf:
        pdf.savefig(perf_fig)
        pdf.savefig(summary_fig)
    
    if not args.no_show:
        print("显示图表...")
        plt.show()
    
    print("完成!")

if __name__ == "__main__":
    main() 