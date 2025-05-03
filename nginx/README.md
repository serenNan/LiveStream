# RTMP 测试服务器使用指南

此目录包含用于测试 CodecHeader 组件的 RTMP 服务器配置和脚本。

## 文件说明

- `nginx.conf` - Nginx RTMP 服务器配置文件
- `mime.types` - MIME 类型定义文件
- `stat.xsl` - RTMP 状态页面样式表
- `start_rtmp_server.sh` - 启动 RTMP 服务器的脚本
- `stop_rtmp_server.sh` - 停止 RTMP 服务器的脚本
- `test_codec_header.sh` - 准备和运行 TestCodecHeader 测试程序的脚本
- `index.html` - Web 测试页面

## 使用方法

### 1. 启动 RTMP 服务器

```bash
cd ~/Projects/LiveStreaming/nginx_rtmp_conf
./start_rtmp_server.sh
```

启动后，RTMP 服务器将在以下地址运行：
- RTMP 地址：`rtmp://localhost:1935/live/stream`
- 状态页面：http://localhost:8080/stat
- 测试页面：http://localhost:8080

### 2. 使用 OBS Studio 推流

1. 下载并安装 [OBS Studio](https://obsproject.com/)
2. 在 OBS 中配置：
   - 打开 OBS Studio
   - 在"设置" > "流"中选择：
     - 服务：自定义
     - 服务器：`rtmp://localhost:1935/live`
     - 串流密钥：`stream`
   - 添加视频源（桌面捕获或窗口捕获）
   - 添加音频源（可选）
   - 点击"开始推流"按钮

### 3. 运行 TestCodecHeader 测试程序

方法一：使用测试脚本（自动修改 RTMP 地址并编译）

```bash
cd ~/Projects/LiveStreaming/nginx_rtmp_conf
./test_codec_header.sh
```

方法二：手动运行

```bash
# 编译
cd ~/Projects/LiveStreaming/build
make TestCodecHeader

# 运行
./tests/TestCodecHeader
```

### 4. 监控和验证

- 通过网页查看 RTMP 状态：http://localhost:8080/stat
- 检查 TestCodecHeader 输出，应该能看到成功解析视频头信息、音频头信息和元数据的消息
- 如果程序能够连接并解析头信息，则表示 CodecHeader 组件工作正常

### 5. 停止 RTMP 服务器

```bash
cd ~/Projects/LiveStreaming/nginx_rtmp_conf
./stop_rtmp_server.sh
```

## 故障排除

1. **RTMP 连接被拒绝**
   - 确认 Nginx 服务器正在运行
   - 检查防火墙设置是否允许 1935 端口通信

2. **OBS 推流错误**
   - 检查 OBS 中的服务器 URL 和串流密钥是否正确
   - 确认 RTMP 服务器已启动

3. **无法访问状态页面**
   - 确认 Nginx 服务器正在运行
   - 检查 8080 端口是否被其他应用占用

4. **TestCodecHeader 连接失败**
   - 确认 OBS 正在成功推流
   - 检查 RTMP URL 是否正确

5. **Nginx 启动失败**
   - 检查配置文件路径是否正确
   - 查看错误日志：`sudo tail -f /usr/local/nginx/logs/error.log` 