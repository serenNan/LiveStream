#pragma once
#include <iostream>
#include "base/LogStream.h"

using namespace tmms::base;

#define RTMP_DEBUG_ON 1

#ifdef RTMP_DEBUG_ON
#define RTMP_TRACE LOG_TRACE << "RTMP::"
#define RTMP_DEBUG LOG_DEBUG << "RTMP::"
#define RTMP_INFO LOG_INFO << "RTMP::"
#elif
#define RTMP_TRACE if(0) LOG_TRACE << "RTMP::"
#define RTMP_DEBUG if(0) LOG_DEBUG << "RTMP::"
#define RTMP_INFO if(0) LOG_INFO << "RTMP::"
#endif

#define RTMP_WARN LOG_WARN
#define RTMP_ERROR LOG_ERROR