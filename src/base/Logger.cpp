#include "base/Logger.h"
#include <iostream>
using namespace tmms::base;

Logger::Logger(const FileLogPtr &log):log_(log)
{
    
}

void Logger::SetLogLevel(const LogLevel &level)
{
    level_ = level;
}

LogLevel Logger::GetLogLevel() const
{
    return level_;
}

void Logger::WriteLog(const std::string &message)
{
    if (log_)
    {
        log_->WriteLog(message);
    }
    else
    {
        std::cout << message << std::endl;
    }
}