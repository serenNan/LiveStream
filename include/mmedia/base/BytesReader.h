#pragma once
#include <stdint.h>

namespace tmms
{
    namespace mm
    {
        /**
         * @brief 字节读取工具类
         *
         * 提供静态方法用于从字节流中读取不同长度的无符号整数，
         * 常用于网络协议解析、二进制数据处理等场景。
         */
        class BytesReader
        {
          public:
            /**
             * @brief 默认构造函数
             */
            BytesReader() = default;

            /**
             * @brief 从字节流中读取8字节（64位）无符号整数（大端序）
             * @param data 指向字节流的指针，至少8字节
             * @return 解析得到的uint64_t值
             */
            static uint64_t ReadUint64T(const char *data);

            /**
             * @brief 从字节流中读取4字节（32位）无符号整数（大端序）
             * @param data 指向字节流的指针，至少4字节
             * @return 解析得到的uint32_t值
             */
            static uint32_t ReadUint32T(const char *data);

            /**
             * @brief 从字节流中读取3字节（24位）无符号整数（大端序）
             * @param data 指向字节流的指针，至少3字节
             * @return 解析得到的uint32_t值
             */
            static uint32_t ReadUint24T(const char *data);

            /**
             * @brief 从字节流中读取2字节（16位）无符号整数（大端序）
             * @param data 指向字节流的指针，至少2字节
             * @return 解析得到的uint16_t值
             */
            static uint16_t ReadUint16T(const char *data);

            /**
             * @brief 从字节流中读取1字节（8位）无符号整数
             * @param data 指向字节流的指针，至少1字节
             * @return 解析得到的uint8_t值
             */
            static uint8_t ReadUint8T(const char *data);

            /**
             * @brief 默认析构函数
             */
            ~BytesReader() = default;
        };
    } // namespace mm
} // namespace tmms
