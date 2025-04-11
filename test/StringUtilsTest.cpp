#include "base/StringUtils.h"
#include "gtest/gtest.h"
using namespace tmms::base;

TEST(StringUtilsTest, StartsWith) {
    EXPECT_TRUE(StringUtils::StartsWith("hello world", "hello"));
    EXPECT_FALSE(StringUtils::StartsWith("hello world", "world"));
    EXPECT_TRUE(StringUtils::StartsWith("hello", ""));
    EXPECT_FALSE(StringUtils::StartsWith("", "hello"));
}

TEST(StringUtilsTest, EndsWith) {
    EXPECT_TRUE(StringUtils::EndsWith("hello world", "world"));
    EXPECT_FALSE(StringUtils::EndsWith("hello world", "hello"));
    EXPECT_TRUE(StringUtils::EndsWith("hello", ""));
    EXPECT_FALSE(StringUtils::EndsWith("", "hello"));
}

TEST(StringUtilsTest, FilePath) {
    EXPECT_EQ(StringUtils::FilePath("/home/user/file.txt"), "/home/user");
    EXPECT_EQ(StringUtils::FilePath("C:\\Users\\file.txt"), "C:\\Users");
    EXPECT_EQ(StringUtils::FilePath("file.txt"), "");
    EXPECT_EQ(StringUtils::FilePath(""), "");
}

TEST(StringUtilsTest, FileNameExt) {
    EXPECT_EQ(StringUtils::FileNameExt("/home/user/file.txt"), "file.txt");
    EXPECT_EQ(StringUtils::FileNameExt("C:\\Users\\file.txt"), "file.txt");
    EXPECT_EQ(StringUtils::FileNameExt("file.txt"), "file.txt");
    EXPECT_EQ(StringUtils::FileNameExt(""), "");
}

TEST(StringUtilsTest, FileName) {
    EXPECT_EQ(StringUtils::FileName("/home/user/file.txt"), "file");
    EXPECT_EQ(StringUtils::FileName("C:\\Users\\file.txt"), "file");
    EXPECT_EQ(StringUtils::FileName("file.txt"), "file");
    EXPECT_EQ(StringUtils::FileName("file"), "file");
    EXPECT_EQ(StringUtils::FileName(""), "");
}

TEST(StringUtilsTest, Extension) {
    EXPECT_EQ(StringUtils::Extension("/home/user/file.txt"), ".txt");
    EXPECT_EQ(StringUtils::Extension("C:\\Users\\file.txt"), ".txt");
    EXPECT_EQ(StringUtils::Extension("file.txt"), ".txt");
    EXPECT_EQ(StringUtils::Extension("file"), "");
    EXPECT_EQ(StringUtils::Extension(""), "");
}

TEST(StringUtilsTest, SplitString) {
    // 正常分割
    std::vector<std::string> result1 = StringUtils::SplitString("a,b,c", ",");
    EXPECT_EQ(result1.size(), 3);
    EXPECT_EQ(result1[0], "a");
    EXPECT_EQ(result1[1], "b");
    EXPECT_EQ(result1[2], "c");

    // 空字符串输入
    std::vector<std::string> result2 = StringUtils::SplitString("", ",");
    EXPECT_EQ(result2.size(), 1);
    EXPECT_EQ(result2[0], "");

    // 空分隔符
    std::vector<std::string> result3 = StringUtils::SplitString("abc", "");
    EXPECT_EQ(result3.size(), 1);
    EXPECT_EQ(result3[0], "abc");

    // 边界条件：分隔符在开头
    std::vector<std::string> result4 = StringUtils::SplitString(",a,b", ",");
    EXPECT_EQ(result4.size(), 3);
    EXPECT_EQ(result4[0], "");
    EXPECT_EQ(result4[1], "a");
    EXPECT_EQ(result4[2], "b");

    // 边界条件：分隔符在结尾
    std::vector<std::string> result5 = StringUtils::SplitString("a,b,", ",");
    EXPECT_EQ(result5.size(), 3);
    EXPECT_EQ(result5[0], "a");
    EXPECT_EQ(result5[1], "b");
    EXPECT_EQ(result5[2], "");

    // 多字符分隔符
    std::vector<std::string> result6 = StringUtils::SplitString("a::b::c", "::");
    EXPECT_EQ(result6.size(), 3);
    EXPECT_EQ(result6[0], "a");
    EXPECT_EQ(result6[1], "b");
    EXPECT_EQ(result6[2], "c");
}