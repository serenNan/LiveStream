#include "FileLog.h"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

using namespace tmms::base;

bool FileLog::Open(const std::string &filePath)
{
    file_path_ = filePath;
    int fd = open(filePath.c_str(), O_WRONLY | O_CREAT | O_APPEND, DEFFILEMODE);
    if (fd < 0)
    {
        std::cerr << "Failed to open log path: " << filePath << std::endl;
        return false;
    }
    fd_ = fd;
    return true;
}

size_t FileLog::WriteLog(const std::string &message)
{
    int fd = fd_ == -1 ? 1 : fd_;
    return ::write(fd, message.c_str(), message.size());
}

void FileLog::Rotate(const std::string &file)
{
    if (file_path_.empty())
    {
        return;
    }
    int ret = ::rename(file_path_.c_str(), file.c_str());
    if (ret != 0)
    {
        std::cerr << "Failed to rename log file: " << file_path_ << " to " << file << std::endl;
    }
    int fd = open(file_path_.c_str(), O_WRONLY | O_CREAT | O_APPEND, DEFFILEMODE);
    if (fd < 0)
    {
        std::cerr << "Failed to open log file: " << file << std::endl;
        return;
    }
    ::dup2(fd, fd_);
    ::close(fd);
}

void FileLog::SetRotateType(RotateType type)
{
    rotate_type_ = type;
}

RotateType FileLog::GetRotateType()
{
    return rotate_type_;
}

int64_t FileLog::GetFileSize() const
{
    return ::lseek64(fd_, 0, SEEK_END);
}

std::string FileLog::GetFilePath() const
{
    return file_path_;
}
