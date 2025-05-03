#!/bin/bash

# 获取脚本所在目录的绝对路径
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CONFIG_PATH="$SCRIPT_DIR/nginx.conf"

# 停止可能已运行的Nginx进程
sudo nginx -s stop 2>/dev/null || true
sleep 1

# 创建必要的目录
sudo mkdir -p /var/log/nginx/rtmp
sudo mkdir -p /usr/share/nginx/html

# 启动Nginx并使用当前目录的配置文件
echo "使用配置文件: $CONFIG_PATH"
sudo nginx -c "$CONFIG_PATH"

echo "RTMP服务器已启动"
echo "RTMP地址: rtmp://localhost:1935/live/stream"
echo "状态页面: http://localhost:8080/stat"
echo "使用OBS等推流软件连接到上述RTMP地址即可"
echo "使用Ctrl+C终止此脚本不会停止Nginx服务"
echo "要停止Nginx服务，请运行: sudo nginx -s stop"

# 保持脚本运行，方便查看日志
sudo tail -f /var/log/nginx/error.log 