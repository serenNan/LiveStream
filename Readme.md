# 直播系统

一个基于RTMP协议的流媒体直播系统，支持视频推流和拉流功能。

## 项目介绍

本项目是一个完整的直播流媒体服务器系统，基于C++17开发，主要实现了以下功能：

- RTMP协议的推流和拉流服务
- 多线程事件处理框架
- 高性能网络通信模块
- 视频GOP缓存管理
- 多媒体编解码处理
- 可配置的日志系统

## 系统架构

系统划分为以下主要模块：

- `base`: 基础工具类，包括配置管理、日志系统、任务管理等
- `network`: 网络模块，包括TCP/UDP服务器和客户端实现
- `mmedia`: 多媒体处理模块，特别是RTMP协议的实现
- `live`: 直播核心服务，包括会话管理、流处理等

## 构建要求

- CMake 3.10+
- C++17兼容的编译器（如GCC 7+）
- Linux系统环境（已在Ubuntu 22.04上测试）

## 构建步骤

```bash
# 克隆仓库
git clone <repository-url>
cd LiveStreaming

# 创建构建目录
mkdir -p build
cd build

# 配置和构建
cmake ..
make

# 安装
make install
```

构建后的可执行文件将被安装到`bin/sbin`目录，配置文件将被安装到`conf`目录。

## 使用方法

### 启动服务器

```bash
cd build
./LiveStreaming
```

或者使用安装后的可执行文件：

```bash
cd bin/sbin
./LiveStreaming
```

### 配置文件

主要配置文件位于`bin/config/config.json`，可以配置以下内容：

- CPU使用情况（线程数、CPU数量）
- 日志设置（级别、文件名、路径）
- 服务设置（监听地址、端口、协议类型）
- 其他目录配置

示例配置：
```json
{
  "name": "tmms server",
  "cpu_start" : 0,
  "threads" : 4,
  "cpus" : 4,
  "log" :
  {
    "level" : "DEBUG",
    "name" : "tmms_server.log",
    "path" : "../logs"
  },
  "service" :
    [
        {
            "addr" : "0.0.0.0",
            "port" : 1935,
            "protocol": "rtmp",
            "transport":"tcp"
        }           
    ],
    "directory" : 
    [
        "../bin/config/publish/"
    ]
}
```

## RTMP推流和拉流

### 使用OBS Studio推流

1. 下载并安装 [OBS Studio](https://obsproject.com/)
2. 在OBS中配置：
   - 打开OBS Studio
   - 在"设置" > "流"中选择：
     - 服务：自定义
     - 服务器：`rtmp://服务器IP:1935/live`
     - 串流密钥：自定义名称（将作为流ID）
   - 添加视频源（桌面捕获或窗口捕获）
   - 添加音频源（可选）
   - 点击"开始推流"按钮

### 使用播放器观看直播

使用支持RTMP协议的播放器（如VLC、ffplay等）播放直播内容：

```bash
# 使用ffplay播放
ffplay rtmp://服务器IP:1935/live/流ID

# 使用VLC播放
vlc rtmp://服务器IP:1935/live/流ID
```

## 与Nginx RTMP模块集成

项目包含一个基于Nginx RTMP模块的测试服务器配置，位于`nginx`目录。

### 启动Nginx RTMP服务器

```bash
cd nginx
./start_rtmp_server.sh
```

启动后，可通过以下方式访问：
- RTMP地址：`rtmp://localhost:1935/live/stream`
- 状态页面：http://localhost:8080/stat
- 测试页面：http://localhost:8080

### 停止Nginx RTMP服务器

```bash
cd nginx
./stop_rtmp_server.sh
```


