#pragma once
#include <memory>
#include <string>
namespace tmms
{
    namespace base
    {
        /**
         * @brief 日志文件切分类型枚举
         */
        enum RotateType
        {
            kRotateNone,   ///< 不进行切分
            kRotateMinute, ///< 每分钟切分（仅作为测试）
            kRotateHour,   ///< 每小时切分
            kRotateDay,    ///< 每天切分
        };
        /**
         * @brief 文件日志类，提供日志文件操作功能
         *
         * 支持日志文件打开、写入、切分等功能
         */
        class FileLog
        {
          public:
            FileLog() = default;
            ~FileLog() = default;
            /**
             * @brief 打开日志文件
             * @param filePath 日志文件路径
             * @return true 打开成功，false 打开失败
             */
            bool Open(const std::string &filePath);
            /**
             * @brief 写入日志
             * @param message 日志消息内容
             * @return 实际写入的字节数
             */
            size_t WriteLog(const std::string &message);
            /**
             * @brief 切分日志文件
             * @param file 新日志文件路径
             */
            void Rotate(const std::string &file);
            /**
             * @brief 设置日志切分类型
             * @param type 切分类型
             */
            void SetRotateType(RotateType type);
            /**
             * @brief 获取当前日志切分类型
             * @return RotateType 当前切分类型
             */
            RotateType GetRotateType();
            /**
             * @brief 获取日志文件大小
             * @return int64_t 文件大小(字节)
             */
            int64_t GetFileSize() const;
            /**
             * @brief 获取日志文件路径
             * @return std::string 文件路径
             */
            std::string GetFilePath() const;

          private:
            int fd_{-1};            ///< 日志文件描述符，-1表示未打开
            std::string file_path_; ///< 当前日志文件路径
            RotateType rotate_type_{kRotateNone};
        };
        using FileLogPtr = std::shared_ptr<FileLog>;
    } // namespace base
} // namespace tmms