#pragma once
#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <cstring>
#include <stdio.h>
#include <string>
#include <vector>

namespace tmms
{
    namespace network
    {
        /// 默认缓冲区大小（2048字节）
        static constexpr size_t kBufferDefaultLength{2048};
        /// 回车换行符
        static constexpr char CRLF[]{"\r\n"};

        /**
         * @brief 消息缓冲区类，用于网络通信中的数据收发
         *
         * MsgBuffer 实现了一个循环缓冲区，支持以下特性：
         * - 自动扩容
         * - 高效的读写操作
         * - 支持多种数据类型的读写
         * - 支持网络字节序的转换
         * - 支持 CRLF 分割的文本协议
         */
        class MsgBuffer
        {
          public:
            /**
             * @brief 构造一个新的消息缓冲区
             * @param len 缓冲区的初始容量，默认为 kBufferDefaultLength (2048字节)
             * @note 缓冲区会根据需要自动扩容
             */
            MsgBuffer(size_t len = kBufferDefaultLength);

            /**
             * @brief 获取当前可读数据的起始位置
             * @return const char* 指向可读数据起始位置的指针
             * @note 该指针在缓冲区扩容时可能失效
             */
            const char *Peek() const
            {
                return begin() + head_;
            }

            /**
             * @brief 获取可写入位置（常量版本）
             * @return const char* 指向可写入位置的常量指针
             */
            const char *BeginWrite() const
            {
                return begin() + tail_;
            }

            /**
             * @brief 获取可写入位置（非常量版本）
             * @return char* 指向可写入位置的指针
             */
            char *BeginWrite()
            {
                return begin() + tail_;
            }

            /**
             * @brief 读取一个字节但不移动读指针
             * @return uint8_t 读取的字节值
             * @throw assert 如果缓冲区可读数据不足1字节则触发断言
             */
            uint8_t PeekInt8() const
            {
                assert(ReadableBytes() >= 1);
                return *(static_cast<const uint8_t *>((void *)Peek()));
            }

            /**
             * @brief 读取一个16位整数但不移动读指针
             * @return uint16_t 网络字节序的16位无符号整数
             * @throw assert 如果缓冲区可读数据不足2字节则触发断言
             */
            uint16_t PeekInt16() const;

            /**
             * @brief 读取一个32位整数但不移动读指针
             * @return uint32_t 网络字节序的32位无符号整数
             * @throw assert 如果缓冲区可读数据不足4字节则触发断言
             */
            uint32_t PeekInt32() const;

            /**
             * @brief 读取一个64位整数但不移动读指针
             * @return uint64_t 网络字节序的64位无符号整数
             * @throw assert 如果缓冲区可读数据不足8字节则触发断言
             */
            uint64_t PeekInt64() const;

            /**
             * @brief 读取指定长度的数据并移动读指针
             * @param len 要读取的字节数
             * @return std::string 包含读取数据的字符串
             * @throw assert 如果缓冲区可读数据不足len字节则触发断言
             */
            std::string Read(size_t len);

            /**
             * @brief 读取一个字节并移动读指针
             * @return uint8_t 读取的字节值
             * @throw assert 如果缓冲区可读数据不足1字节则触发断言
             */
            uint8_t ReadInt8();

            /**
             * @brief 读取一个16位整数并移动读指针
             * @return uint16_t 网络字节序的16位无符号整数
             * @throw assert 如果缓冲区可读数据不足2字节则触发断言
             */
            uint16_t ReadInt16();

            /**
             * @brief 读取一个32位整数并移动读指针
             * @return uint32_t 网络字节序的32位无符号整数
             * @throw assert 如果缓冲区可读数据不足4字节则触发断言
             */
            uint32_t ReadInt32();

            /**
             * @brief 读取一个64位整数并移动读指针
             * @return uint64_t 网络字节序的64位无符号整数
             * @throw assert 如果缓冲区可读数据不足8字节则触发断言
             */
            uint64_t ReadInt64();

            /**
             * @brief 与另一个缓冲区交换内容
             * @param buf 要交换的目标缓冲区
             * @note 这是一个快速操作，不涉及实际的数据复制
             */
            void Swap(MsgBuffer &buf) noexcept;

            /**
             * @brief 获取当前可读数据的字节数
             * @return size_t 可读数据的字节数
             */
            size_t ReadableBytes() const
            {
                return tail_ - head_;
            }

            /**
             * @brief 获取当前可写入的空闲空间大小
             * @return size_t 可写入的字节数
             */
            size_t WritableBytes() const
            {
                return buffer_.size() - tail_;
            }

            /**
             * @brief 追加数据到缓冲区
             * @note 提供了多个重载版本以支持不同的数据类型
             */
            void Append(const MsgBuffer &buf);

            /**
             * @brief 追加定长字符数组到缓冲区
             * @tparam N 数组长度
             * @param buf 要追加的字符数组
             * @throw assert 如果字符数组不是以null结尾则触发断言
             */
            template <int N> void Append(const char (&buf)[N])
            {
                assert(strnlen(buf, N) == N - 1);
                Append(buf, N - 1);
            }

            /**
             * @brief 追加指定长度的数据到缓冲区
             * @param buf 要追加的数据的指针
             * @param len 要追加的数据长度
             */
            void Append(const char *buf, size_t len);

            /**
             * @brief 追加字符串到缓冲区
             * @param buf 要追加的字符串
             */
            void Append(const std::string &buf)
            {
                Append(buf.c_str(), buf.length());
            }

            /**
             * @brief 追加一个字节到缓冲区
             * @param b 要追加的字节值
             */
            void AppendInt8(const uint8_t b)
            {
                Append(static_cast<const char *>((void *)&b), 1);
            }

            /**
             * @brief 追加一个16位整数到缓冲区（网络字节序）
             * @param s 要追加的16位整数
             */
            void AppendInt16(const uint16_t s);

            /**
             * @brief 追加一个32位整数到缓冲区（网络字节序）
             * @param i 要追加的32位整数
             */
            void AppendInt32(const uint32_t i);

            /**
             * @brief 追加一个64位整数到缓冲区（网络字节序）
             * @param l 要追加的64位整数
             */
            void AppendInt64(const uint64_t l);

            /**
             * @brief 在缓冲区头部添加数据
             * @param buf 要添加的数据的指针
             * @param len 要添加的数据长度
             * @note 此操作可能导致内存重新分配
             */
            void AddInFront(const char *buf, size_t len);

            /**
             * @brief 在缓冲区头部添加一个字节
             * @param b 要添加的字节值
             */
            void AddInFrontInt8(const uint8_t b)
            {
                AddInFront(static_cast<const char *>((void *)&b), 1);
            }

            /**
             * @brief 在缓冲区头部添加一个16位整数（网络字节序）
             * @param s 要添加的16位整数
             */
            void AddInFrontInt16(const uint16_t s);

            /**
             * @brief 在缓冲区头部添加一个32位整数（网络字节序）
             * @param i 要添加的32位整数
             */
            void AddInFrontInt32(const uint32_t i);

            /**
             * @brief 在缓冲区头部添加一个64位整数（网络字节序）
             * @param l 要添加的64位整数
             */
            void AddInFrontInt64(const uint64_t l);

            /**
             * @brief 清空缓冲区中的所有数据
             * @note 不会释放已分配的内存
             */
            void RetrieveAll();

            /**
             * @brief 从缓冲区中移除指定长度的数据
             * @param len 要移除的字节数
             * @throw assert 如果可读数据不足len字节则触发断言
             */
            void Retrieve(size_t len);

            /**
             * @brief 从文件描述符读取数据
             * @param fd 文件描述符
             * @param retErrno 用于存储错误码的指针
             * @return ssize_t 读取的字节数，出错时返回-1
             * @note 主要用于网络socket的读取操作
             */
            ssize_t ReadFd(int fd, int *retErrno);

            /**
             * @brief 移除直到指定位置的数据
             * @param end 目标位置的指针
             * @throw assert 如果end指针不在有效范围内则触发断言
             */
            void RetrieveUntil(const char *end)
            {
                assert(Peek() <= end);
                assert(end <= BeginWrite());
                Retrieve(end - Peek());
            }

            /**
             * @brief 查找CRLF标记
             * @return const char* CRLF的位置，如果未找到则返回NULL
             * @note 用于处理基于行的文本协议
             */
            const char *FindCRLF() const
            {
                const char *crlf = std::search(Peek(), BeginWrite(), CRLF, CRLF + 2);
                return crlf == BeginWrite() ? NULL : crlf;
            }

            /**
             * @brief 确保缓冲区有足够的可写空间
             * @param len 需要的空间大小（字节）
             * @note 如果空间不足会自动扩容
             */
            void EnsureWritableBytes(size_t len);

            /**
             * @brief 标记已写入指定长度的数据
             * @param len 已写入的字节数
             * @throw assert 如果可写空间不足len字节则触发断言
             */
            void HasWritten(size_t len)
            {
                assert(len <= WritableBytes());
                tail_ += len;
            }

            /**
             * @brief 回退写入位置
             * @param offset 要回退的字节数
             * @throw assert 如果可读数据不足offset字节则触发断言
             */
            void Unwrite(size_t offset)
            {
                assert(ReadableBytes() >= offset);
                tail_ -= offset;
            }

            /**
             * @brief 访问缓冲区中的数据（常量版本）
             * @param offset 相对于读取位置的偏移量
             * @return const char& 指定位置的字符引用
             * @throw assert 如果偏移量超出可读范围则触发断言
             */
            const char &operator[](size_t offset) const
            {
                assert(ReadableBytes() >= offset);
                return Peek()[offset];
            }

            /**
             * @brief 访问缓冲区中的数据（非常量版本）
             * @param offset 相对于读取位置的偏移量
             * @return char& 指定位置的字符引用
             * @throw assert 如果偏移量超出可读范围则触发断言
             */
            char &operator[](size_t offset)
            {
                assert(ReadableBytes() >= offset);
                return begin()[head_ + offset];
            }

          private:
            size_t head_;              ///< 读取位置的索引
            size_t initCap_;           ///< 初始分配的缓冲区容量
            std::vector<char> buffer_; ///< 存储数据的底层容器
            size_t tail_;              ///< 写入位置的索引

            /**
             * @brief 获取缓冲区起始位置（常量版本）
             * @return const char* 指向缓冲区起始位置的常量指针
             */
            const char *begin() const
            {
                return &buffer_[0];
            }

            /**
             * @brief 获取缓冲区起始位置（非常量版本）
             * @return char* 指向缓冲区起始位置的指针
             */
            char *begin()
            {
                return &buffer_[0];
            }
        };

        /**
         * @brief 交换两个消息缓冲区的内容
         * @param one 第一个缓冲区
         * @param two 第二个缓冲区
         */
        inline void swap(MsgBuffer &one, MsgBuffer &two) noexcept
        {
            one.Swap(two);
        }

        /**
         * @brief 将64位整数转换为网络字节序
         * @param n 要转换的整数
         * @return uint64_t 网络字节序的整数
         */
        inline uint64_t hton64(uint64_t n)
        {
            static const int one = 1;
            static const char sig = *(char *)&one;
            if (sig == 0)
                return n; // 对于大端机器直接返回输入值
            char *ptr = reinterpret_cast<char *>(&n);
            std::reverse(ptr, ptr + sizeof(uint64_t));
            return n;
        }

        /**
         * @brief 将网络字节序的64位整数转换为主机字节序
         * @param n 要转换的网络字节序整数
         * @return uint64_t 主机字节序的整数
         */
        inline uint64_t ntoh64(uint64_t n)
        {
            return hton64(n);
        }
    } // namespace network
} // namespace tmms

namespace std
{
    /**
     * @brief 特化 std::swap 以支持 MsgBuffer 的交换操作
     * @param one 第一个缓冲区
     * @param two 第二个缓冲区
     */
    template <>
    inline void swap(tmms::network::MsgBuffer &one, tmms::network::MsgBuffer &two) noexcept
    {
        one.Swap(two);
    }
} // namespace std