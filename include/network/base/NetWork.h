#pragma once

#include "base/LogStream.h"

using namespace tmms::base;

/// 网络模块日志宏定义，基于基础日志模块实现
#define NETWORK_TRACE LOG_TRACE ///< 跟踪级别日志
#define NETWORK_DEBUG LOG_DEBUG ///< 调试级别日志
#define NETWORK_INFO LOG_INFO   ///< 信息级别日志
#define NETWORK_WARN LOG_WARN   ///< 警告级别日志
#define NETWORK_ERROR LOG_ERROR ///< 错误级别日志