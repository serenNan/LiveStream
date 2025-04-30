#pragma once
#include "AMFAny.h"

namespace tmms
{
    namespace mm
    {
        /**
         * @class AMFString
         * @brief 表示AMF数据中的字符串类型
         *
         * AMFString类用于表示AMF数据中的字符串值，支持UTF-8编码的字符串。
         * 在RTMP通信中常用于表示各种文本数据，如媒体名称、事件信息等。
         * 继承自AMFAny基类，实现了特定于字符串类型的序列化和反序列化功能。
         */
        class AMFString : public AMFAny
        {
          public:
            /**
             * @brief 构造函数，初始化带名称的AMFString对象
             * @param name AMF数据的名称
             */
            AMFString(const std::string &name);

            /**
             * @brief 默认构造函数，创建一个无名称的AMFString对象
             */
            AMFString();

            /**
             * @brief 解码AMF字符串类型数据
             * @param data 要解码的二进制数据
             * @param size 数据大小
             * @param has 是否已包含类型标记，默认为false
             * @return 成功解码的字节数
             *
             * 从给定的数据缓冲区解析AMF字符串，并存储解析结果
             */
            int Decode(const char *data, int size, bool has = false) override;

            /**
             * @brief 检查是否为字符串类型
             * @return 总是返回true，表示这是一个字符串类型对象
             */
            bool IsString() override;

            /**
             * @brief 获取字符串值
             * @return 字符串值的常量引用
             */
            const std::string &String() override;

            /**
             * @brief 输出调试信息
             *
             * 输出该AMFString对象的详细信息，包括名称和字符串内容
             */
            void Dump() const override;

            /**
             * @brief 析构函数
             */
            ~AMFString();

          private:
            std::string string_; ///< 存储解码后的字符串值
        };
    } // namespace mm
} // namespace tmms