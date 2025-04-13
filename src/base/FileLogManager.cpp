#include "FileLogManager.h"
#include "FileLog.h"
#include "StringUtils.h"
#include "TTime.h"
#include <ctime>
#include <sstream>

using namespace tmms::base;

namespace
{
    static tmms::base::FileLogPtr file_log_nullptr;
}

void FileLogManager::OnCheck()
{
    bool day_change = false;
    bool hour_change = false;
    bool minute_change = false;
    int year = 0, mouth = 0, day = -1, hour = -1, minute = 0, second = 0;
    TTime::Now(year, mouth, day, hour, minute, second);

    if (last_day_ == -1)
    {
        last_year_ = year;
        last_month_ = mouth;
        last_day_ = day;
        last_hour_ = hour;
        last_minute_ = minute;
    }
    if (last_day_ != day)
    {
        day_change = true;
    }
    if (last_hour_ != hour)
    {
        hour_change = true;
    }
    if (last_minute_ != minute)
    {
        minute_change = true;
    }
    if (!day_change && !hour_change && !minute_change)
    {
        return;
    }
    std::lock_guard<std::mutex> lock(lock_);
    for (auto &it : logs_)
    {
        if (minute_change && it.second->GetRotateType() == RotateType::kRotateMinute)
        {
            RotateMinutes(it.second);
        }
        if (hour_change && it.second->GetRotateType() == RotateType::kRotateHour)
        {
            RotateHours(it.second);
        }
        if (day_change && it.second->GetRotateType() == RotateType::kRotateDay)
        {
            RotateDays(it.second);
        }
    }
    last_year_ = year;
    last_month_ = mouth;
    last_day_ = day;
    last_hour_ = hour;
}

FileLogPtr FileLogManager::GetFileLog(const std::string &fileName)
{
    std::lock_guard<std::mutex> lock(lock_);
    auto it = logs_.find(fileName);
    if (it != logs_.end())
    {
        return it->second;
    }

    FileLogPtr log = std::make_shared<FileLog>();
    if (!log->Open(fileName))
    {
        return file_log_nullptr;
    }

    logs_.emplace(fileName, log);
    return log;
}

void FileLogManager::RemoveFileLog(const FileLogPtr &log)
{
    std::lock_guard<std::mutex> lock(lock_);
    logs_.erase(log->GetFilePath());
}

void FileLogManager::RotateDays(const FileLogPtr &file)
{
    if (file->GetFileSize() > 0)
    {
        char buf[128] = {0};
        sprintf(buf, "_%04d-%02d-%02d", last_year_, last_month_, last_day_);
        std::string filePath = file->GetFilePath();
        std::string newFilePath = StringUtils::FilePath(filePath);
        std::string newFileName = StringUtils::FileName(filePath);
        std::string newFileExt = StringUtils::Extension(filePath);

        std::ostringstream oss;
        oss << newFilePath << newFileName << "." << buf << newFileExt;
        file->Rotate(oss.str());
    }
}

void FileLogManager::RotateHours(const FileLogPtr &file)
{
    if (file->GetFileSize() > 0)
    {
        char buf[128] = {0};
        sprintf(buf, "_%04d-%02d-%02dT%02d", last_year_, last_month_, last_day_, last_hour_);
        std::string filePath = file->GetFilePath();
        std::string newFilePath = StringUtils::FilePath(filePath);
        std::string newFileName = StringUtils::FileName(filePath);
        std::string newFileExt = StringUtils::Extension(filePath);

        std::ostringstream oss;
        oss << newFilePath << newFileName << "." << buf << newFileExt;
        file->Rotate(oss.str());
    }
}

void FileLogManager::RotateMinutes(const FileLogPtr &file)
{
    if (file->GetFileSize() > 0)
    {
        char buf[128] = {0};
        sprintf(buf, "_%04d-%02d-%02dT%02d%02d", last_year_, last_month_, last_day_, last_hour_,last_minute_);
        std::string filePath = file->GetFilePath();
        std::string newFilePath = StringUtils::FilePath(filePath);
        std::string newFileName = StringUtils::FileName(filePath);
        std::string newFileExt = StringUtils::Extension(filePath);

        std::ostringstream oss;
        oss << newFilePath << newFileName << "." << buf << newFileExt;
        file->Rotate(oss.str());
    }
}