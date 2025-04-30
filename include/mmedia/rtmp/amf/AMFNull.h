#pragma once
#include "AMFAny.h"

namespace tmms
{
    namespace mm
    {
        /**
         * @class AMFNull
         * @brief 表示AMF数据中的空类型
         *
         * AMFNull类用于表示AMF数据中的null值，在Flash和RTMP通信中用来表示空对象或空引用。
         * 继承自AMFAny基类，实现了特定于Null类型的序列化和反序列化功能。
         */
        class AMFNull : public AMFAny
        {
          public:
            /**
             * @brief 构造函数，初始化带名称的AMFNull对象
             * @param name AMF数据的名称
             */
            AMFNull(const std::string &name);

            /**
             * @brief 默认构造函数，创建一个无名称的AMFNull对象
             */
            AMFNull();

            /**
             * @brief 解码AMF空类型数据
             * @param data 要解码的二进制数据
             * @param size 数据大小
             * @param has 是否已包含类型标记，默认为false
             * @return 成功解码的字节数
             *
             * 从给定的数据缓冲区解析AMF null类型数据，并设置内部状态
             */
            int Decode(const char *data, int size, bool has = false) override;

            /**
             * @brief 检查是否为空类型
             * @return 总是返回true，表示这是一个空类型对象
             */
            bool IsNull() override;

            /**
             * @brief 输出调试信息
             *
             * 输出该AMFNull对象的详细信息，包括名称和类型
             */
            void Dump() const override;

            /**
             * @brief 析构函数
             */
            ~AMFNull();

          private:
            bool b_{false}; ///< 标识是否为null的内部标志
        };
    } // namespace mm
} // namespace tmms