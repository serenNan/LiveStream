#include "base/Logger.h"
#include <iostream>
using namespace tmms::base;

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
    std::cout << message << std::endl;
}