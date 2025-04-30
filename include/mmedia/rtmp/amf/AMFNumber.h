#pragma once
#include "AMFAny.h"

namespace tmms
{
    namespace mm
    {
        /**
         * @class AMFNumber
         * @brief 表示AMF数据中的数字类型
         *
         * AMFNumber类用于表示AMF数据中的数字值，使用IEEE-754双精度浮点数格式（64位）。
         * 在RTMP通信中常用于表示各种数值数据，如时间戳、音量、带宽等参数。
         * 继承自AMFAny基类，实现了特定于数字类型的序列化和反序列化功能。
         */
        class AMFNumber : public AMFAny
        {
          public:
            /**
             * @brief 构造函数，初始化带名称的AMFNumber对象
             * @param name AMF数据的名称
             */
            AMFNumber(const std::string &name);

            /**
             * @brief 默认构造函数，创建一个无名称的AMFNumber对象
             */
            AMFNumber();

            /**
             * @brief 解码AMF数字类型数据
             * @param data 要解码的二进制数据
             * @param size 数据大小
             * @param has 是否已包含类型标记，默认为false
             * @return 成功解码的字节数
             *
             * 从给定的数据缓冲区解析AMF数字，并存储解析结果
             */
            int Decode(const char *data, int size, bool has = false) override;

            /**
             * @brief 检查是否为数字类型
             * @return 总是返回true，表示这是一个数字类型对象
             */
            bool IsNumber() override;

            /**
             * @brief 获取数字值
             * @return 双精度浮点数值
             */
            double Number() override;

            /**
             * @brief 输出调试信息
             *
             * 输出该AMFNumber对象的详细信息，包括名称和数字值
             */
            void Dump() const override;

            /**
             * @brief 析构函数
             */
            ~AMFNumber();

          private:
            double number_{0.0f}; ///< 存储解码后的数字值，默认初始化为0.0
        };
    } // namespace mm
} // namespace tmms