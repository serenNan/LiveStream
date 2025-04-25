#include "StringUtils.h"

using namespace tmms::base;

bool StringUtils::StartsWith(const std::string &s, const std::string &sub)
{
    if (sub.empty())
    {
        return true;
    }
    if (s.length() < sub.length())
    {
        return false;
    }
    return s.compare(0, sub.length(), sub) == 0;
}

bool StringUtils::EndsWith(const std::string &s, const std::string &sub)
{
    if (sub.empty())
    {
        return true;
    }
    if (s.length() < sub.length())
    {
        return false;
    }
    return s.compare(s.length() - sub.length(), sub.length(), sub) == 0;
}

std::string StringUtils::FilePath(const std::string &path)
{
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos)
    {
        return path.substr(0, pos);
    }
    else
    {
        return "";
    }
}

std::string StringUtils::FileNameExt(const std::string &path)
{
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos)
    {
        return path;
    }
    return path.substr(pos + 1);
}

std::string StringUtils::FileName(const std::string &path)
{
    std::string filename = FileNameExt(path);
    size_t pos = filename.find_last_of('.');
    if (pos == std::string::npos)
    {
        return filename;
    }
    return filename.substr(0, pos);
}

std::string StringUtils::Extension(const std::string &path)
{
    std::string filename = FileNameExt(path);
    size_t pos = filename.find_last_of('.');
    if (pos == std::string::npos)
    {
        return "";
    }
    return filename.substr(pos);
}

std::vector<std::string> StringUtils::SplitString(const std::string &s,
                                                  const std::string &delimiter)
{
    std::vector<std::string> result;
    if (delimiter.empty())
    {
        result.push_back(s);
        return result;
    }

    size_t start = 0;
    size_t end = s.find(delimiter);
    while (end != std::string::npos)
    {
        result.push_back(s.substr(start, end - start));
        start = end + delimiter.length();
        end = s.find(delimiter, start);
    }
    result.push_back(s.substr(start));
    return result;
}

std::vector<std::string> StringUtils::SplitStringWithFSM(const std::string &s, const char delimiter)
{
    enum
    {
        kStateInit = 0,      // 初始状态
        kStateNormal = 1,    // 正常状态，处理非分隔符字符
        kStateDelimiter = 2, // 分隔符状态，处理分隔符
        kStateEnd = 3        // 结束状态
    };

    std::vector<std::string> result;

    if (s.empty())
    {
        return result;
    }

    int state = kStateInit;
    std::string tmp;

    for (size_t pos = 0; pos < s.size(); ++pos)
    {
        char ch = s[pos];

        switch (state)
        {
        case kStateInit:
            if (ch == delimiter)
            {
                state = kStateDelimiter;
            }
            else
            {
                tmp.push_back(ch);
                state = kStateNormal;
            }
            break;

        case kStateNormal:
            if (ch == delimiter)
            {
                if (!tmp.empty())
                {
                    result.push_back(tmp);
                    tmp.clear();
                }
                state = kStateDelimiter;
            }
            else
            {
                tmp.push_back(ch);
            }
            break;

        case kStateDelimiter:
            if (ch == delimiter)
            {
                // 连续的分隔符，保持当前状态
            }
            else
            {
                tmp.push_back(ch);
                state = kStateNormal;
            }
            break;
        }
    }

    // 处理最后的状态
    if (!tmp.empty())
    {
        result.push_back(tmp);
    }

    return result;
}