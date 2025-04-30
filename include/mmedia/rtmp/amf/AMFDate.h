#pragma once
#include "AMFAny.h"

namespace tmms
{
    namespace mm
    {
        /**
         * @class AMFDate
         * @brief 表示AMF数据中的日期类型
         *
         * AMFDate类用于表示AMF数据中的日期和时间值，由一个毫秒精度的UTC时间戳
         * 和一个表示时区偏移量的整数组成。时间戳是自1970年1月1日以来的毫秒数。
         * 在RTMP通信中用于表示与时间相关的信息，如创建时间、修改时间等。
         * 继承自AMFAny基类，实现了特定于日期类型的序列化和反序列化功能。
         */
        class AMFDate : public AMFAny
        {
          public:
            /**
             * @brief 构造函数，初始化带名称的AMFDate对象
             * @param name AMF数据的名称
             */
            AMFDate(const std::string &name);

            /**
             * @brief 默认构造函数，创建一个无名称的AMFDate对象
             */
            AMFDate();

            /**
             * @brief 解码AMF日期类型数据
             * @param data 要解码的二进制数据
             * @param size 数据大小
             * @param has 是否已包含类型标记，默认为false
             * @return 成功解码的字节数
             *
             * 从给定的数据缓冲区解析AMF日期，包括UTC时间戳和时区偏移量
             */
            int Decode(const char *data, int size, bool has = false) override;

            /**
             * @brief 检查是否为日期类型
             * @return 总是返回true，表示这是一个日期类型对象
             */
            bool IsDate() override;

            /**
             * @brief 获取日期值（UTC时间戳）
             * @return 表示日期的双精度浮点数（UTC时间戳，毫秒）
             */
            double Date() override;

            /**
             * @brief 输出调试信息
             *
             * 输出该AMFDate对象的详细信息，包括名称、UTC时间戳和时区偏移量
             */
            void Dump() const override;

            /**
             * @brief 获取时区偏移量
             * @return 时区偏移量，以分钟为单位
             *
             * 正值表示UTC以东的时区，负值表示UTC以西的时区
             */
            int16_t UtcOffset() const;

            /**
             * @brief 析构函数
             */
            ~AMFDate();

          private:
            double utc_{0.0f};      ///< 存储UTC时间戳，以毫秒为单位
            int16_t utc_offset_{0}; ///< 存储时区偏移量，以分钟为单位
        };
    } // namespace mm
} // namespace tmms