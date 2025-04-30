#pragma once
#include "AMFAny.h"

namespace tmms
{
    namespace mm
    {
        /**
         * @class AMFBoolean
         * @brief 表示AMF数据中的布尔类型
         *
         * AMFBoolean类用于表示AMF数据中的布尔值，可以是true或false。
         * 在RTMP通信中常用于表示各种二元状态，如是否静音、是否可见等。
         * 继承自AMFAny基类，实现了特定于布尔类型的序列化和反序列化功能。
         */
        class AMFBoolean : public AMFAny
        {
          public:
            /**
             * @brief 构造函数，初始化带名称的AMFBoolean对象
             * @param name AMF数据的名称
             */
            AMFBoolean(const std::string &name);

            /**
             * @brief 默认构造函数，创建一个无名称的AMFBoolean对象
             */
            AMFBoolean();

            /**
             * @brief 解码AMF布尔类型数据
             * @param data 要解码的二进制数据
             * @param size 数据大小
             * @param has 是否已包含类型标记，默认为false
             * @return 成功解码的字节数
             *
             * 从给定的数据缓冲区解析AMF布尔值，并存储解析结果
             */
            int Decode(const char *data, int size, bool has = false) override;

            /**
             * @brief 检查是否为布尔类型
             * @return 总是返回true，表示这是一个布尔类型对象
             */
            bool IsBoolean() override;

            /**
             * @brief 获取布尔值
             * @return 布尔值
             */
            bool Boolean() override;

            /**
             * @brief 输出调试信息
             *
             * 输出该AMFBoolean对象的详细信息，包括名称和布尔值
             */
            void Dump() const override;

            /**
             * @brief 析构函数
             */
            ~AMFBoolean();

          private:
            bool b_{false}; ///< 存储解码后的布尔值，默认初始化为false
        };
    } // namespace mm
} // namespace tmms