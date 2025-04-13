#pragma once
#include <string>
#include <memory>
namespace tmms
{
    namespace base
    {
        enum RotateType
        {
            kRotateNone,
            kRotateMinute,
            kRotateHour,
            kRotateDay,
        };
        class FileLog
        {
          public:
            FileLog() = default;
            ~FileLog() = default;
            bool Open(const std::string &filePath);
            size_t WriteLog(const std::string &message);
            void Rotate(const std::string &file);
            void SetRotateType(RotateType type);
            RotateType GetRotateType();
            int64_t GetFileSize() const;
            std::string GetFilePath() const;

          private:
            int fd_{-1};
            std::string file_path_;
            RotateType rotate_type_{kRotateNone};
        };
        using FileLogPtr = std::shared_ptr<FileLog>;
    } // namespace base
} // namespace tmms