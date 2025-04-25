#pragma once

#include <string>
#include <vector>

namespace tmms
{
    namespace base
    {
        /**
         * @brief 字符串工具类，提供常用的字符串操作方法
         */
        class StringUtils
        {
          public:
            /**
             * @brief 检查字符串是否以指定子串开头
             * @param s 原字符串
             * @param sub 要检查的子串
             * @return bool 是否以子串开头
             */
            static bool StartsWith(const std::string &s, const std::string &sub);

            /**
             * @brief 检查字符串是否以指定子串结尾
             * @param s 原字符串
             * @param sub 要检查的子串
             * @return bool 是否以子串结尾
             */
            static bool EndsWith(const std::string &s, const std::string &sub);

            /**
             * @brief 获取文件路径中的目录部分
             * @param path 文件路径
             * @return std::string 目录路径
             */
            static std::string FilePath(const std::string &path);

            /**
             * @brief 获取文件路径中的文件名(包含扩展名)
             * @param path 文件路径
             * @return std::string 文件名(包含扩展名)
             */
            static std::string FileNameExt(const std::string &path);

            /**
             * @brief 获取文件路径中的文件名(不包含扩展名)
             * @param path 文件路径
             * @return std::string 文件名(不包含扩展名)
             */
            static std::string FileName(const std::string &path);

            /**
             * @brief 获取文件路径中的扩展名
             * @param path 文件路径
             * @return std::string 文件扩展名
             */
            static std::string Extension(const std::string &path);

            /**
             * @brief 分割字符串
             * @param s 原字符串
             * @param delimiter 分隔符
             * @return std::vector<std::string> 分割后的字符串数组
             */
            static std::vector<std::string> SplitString(const std::string &s,
                                                        const std::string &delimiter);

            /**
             * @brief 使用有限状态机分割字符串
             * @param s 原字符串
             * @param delimiter 分隔符
             * @return std::vector<std::string> 分割后的字符串数组
             */
            static std::vector<std::string> SplitStringWithFSM(const std::string &s,
                                                               const char delimiter);
        };
    } // namespace base
} // namespace tmms