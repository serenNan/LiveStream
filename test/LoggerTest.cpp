#include "gtest/gtest.h"
#include "base/Logger.h"
#include "base/LogStream.h"
using namespace tmms::base;

TEST(LoggerTest, BasicLogging)
{
    tmms::base::g_logger = new Logger();
    tmms::base::g_logger->SetLogLevel(tmms::base::kTrace);
    
    LOG_TRACE << "This is a trace message";
    LOG_DEBUG << "This is a debug message";
    LOG_INFO << "This is an info message";
    LOG_WARN << "This is a warning message";
    LOG_ERROR << "This is an error message";
}

