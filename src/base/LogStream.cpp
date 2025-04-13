#include "LogStream.h"
#include "TTime.h"
#include <cstring>
#include <sched.h>
#include <sys/syscall.h>
#include <unistd.h>

using namespace tmms::base;
Logger *tmms::base::g_logger = nullptr;

static thread_local pid_t thread_id = 0;
const char *log_strings[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR",
};

LogStream::LogStream(Logger *logger, const char *file, int line, LogLevel level, const char *func)
    : logger_(logger)
{
    const char *file_name = strrchr(file, '/');
    if (file_name)
    {
        file_name++;
    }
    else
    {
        file_name = file;
    }
    stream_ << TTime::ISOTime() << " ";
    if (thread_id == 0)
    {
        thread_id = static_cast<pid_t>(syscall(SYS_gettid));
    }
    stream_ << thread_id << " ";
    stream_ << log_strings[level] << " ";
    stream_ << "[" << file_name << ":" << line << "] ";
    if (func)
    {
        stream_ << "[" << func << "] ";
    }
}

LogStream::~LogStream()
{
    stream_ << "\n";
    logger_->WriteLog(stream_.str());
}
