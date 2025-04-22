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

        static constexpr size_t kBufferDefaultLength{2048};
        static constexpr char CRLF[]{"\r\n"};

        /**
         * @brief 该类表示用于发送和接收数据的内存缓冲区。
         *
         */
        class MsgBuffer
        {
          public:
            /**
             * @brief 构造一个新的消息缓冲区实例。
             *
             * @param len 缓冲区的初始大小。
             */
            MsgBuffer(size_t len = kBufferDefaultLength);

            /**
             * @brief 获取缓冲区的起始位置。
             *
             * @return const char*
             */
            const char *Peek() const
            {
                return begin() + head_;
            }

            /**
             * @brief 获取缓冲区中可以写入新数据的末尾位置。
             *
             * @return const char*
             */
            const char *BeginWrite() const
            {
                return begin() + tail_;
            }

            /**
             * @brief 获取缓冲区中可以写入新数据的末尾位置。
             *
             * @return const char*
             */
            char *BeginWrite()
            {
                return begin() + tail_;
            }

            /**
             * @brief 从缓冲区获取一个字节值。
             *
             * @return uint8_t
             */
            uint8_t PeekInt8() const
            {
                assert(ReadableBytes() >= 1);
                return *(static_cast<const uint8_t *>((void *)Peek()));
            }

            /**
             * @brief 从缓冲区获取一个无符号短整型值。
             *
             * @return uint16_t
             */
            uint16_t PeekInt16() const;

            /**
             * @brief 从缓冲区获取一个无符号整型值。
             *
             * @return uint32_t
             */
            uint32_t PeekInt32() const;

            /**
             * @brief 从缓冲区获取一个无符号长整型值。
             *
             * @return uint64_t
             */
            uint64_t PeekInt64() const;

            /**
             * @brief 从缓冲区获取并移除一些字节。
             *
             * @param len
             * @return std::string
             */
            std::string Read(size_t len);

            /**
             * @brief 从缓冲区获取并移除一个字节值。
             *
             * @return uint8_t
             */
            uint8_t ReadInt8();

            /**
             * @brief 从缓冲区获取并移除一个无符号短整型值。
             *
             * @return uint16_t
             */
            uint16_t ReadInt16();

            /**
             * @brief 从缓冲区获取并移除一个无符号整型值。
             *
             * @return uint32_t
             */
            uint32_t ReadInt32();

            /**
             * @brief 从缓冲区获取并移除一个无符号长整型值。
             *
             * @return uint64_t
             */
            uint64_t ReadInt64();

            /**
             * @brief 与另一个缓冲区交换内容。
             *
             * @param buf
             */
            void Swap(MsgBuffer &buf) noexcept;

            /**
             * @brief 返回缓冲区中数据的大小。
             *
             * @return size_t
             */
            size_t ReadableBytes() const
            {
                return tail_ - head_;
            }

            /**
             * @brief 返回缓冲区中空闲部分的大小
             *
             * @return size_t
             */
            size_t WritableBytes() const
            {
                return buffer_.size() - tail_;
            }

            /**
             * @brief 向缓冲区追加新数据。
             *
             */
            void Append(const MsgBuffer &buf);
            template <int N> void Append(const char (&buf)[N])
            {
                assert(strnlen(buf, N) == N - 1);
                Append(buf, N - 1);
            }
            void Append(const char *buf, size_t len);
            void Append(const std::string &buf)
            {
                Append(buf.c_str(), buf.length());
            }

            /**
             * @brief 向缓冲区末尾追加一个字节值。
             *
             * @param b
             */
            void AppendInt8(const uint8_t b)
            {
                Append(static_cast<const char *>((void *)&b), 1);
            }

            /**
             * @brief 向缓冲区末尾追加一个无符号短整型值。
             *
             * @param s
             */
            void AppendInt16(const uint16_t s);

            /**
             * @brief 向缓冲区末尾追加一个无符号整型值。
             *
             * @param i
             */
            void AppendInt32(const uint32_t i);

            /**
             * @brief 向缓冲区末尾追加一个无符号长整型值。
             *
             * @param l
             */
            void AppendInt64(const uint64_t l);

            /**
             * @brief 在缓冲区开头添加新数据。
             *
             * @param buf
             * @param len
             */
            void AddInFront(const char *buf, size_t len);

            /**
             * @brief 在缓冲区开头添加一个字节值。
             *
             * @param b
             */
            void AddInFrontInt8(const uint8_t b)
            {
                AddInFront(static_cast<const char *>((void *)&b), 1);
            }

            /**
             * @brief 在缓冲区开头添加一个无符号短整型值。
             *
             * @param s
             */
            void AddInFrontInt16(const uint16_t s);

            /**
             * @brief 在缓冲区开头添加一个无符号整型值。
             *
             * @param i
             */
            void AddInFrontInt32(const uint32_t i);

            /**
             * @brief 在缓冲区开头添加一个无符号长整型值。
             *
             * @param l
             */
            void AddInFrontInt64(const uint64_t l);

            /**
             * @brief 移除缓冲区中的所有数据。
             *
             */
            void RetrieveAll();

            /**
             * @brief 移除缓冲区中的一些字节。
             *
             * @param len
             */
            void Retrieve(size_t len);

            /**
             * @brief 从文件描述符读取数据并放入缓冲区。
             *
             * @param fd 文件描述符，通常是一个socket。
             * @param retErrno 读取时的错误代码。
             * @return ssize_t 从文件描述符读取的字节数，发生错误时返回-1。
             */
            ssize_t ReadFd(int fd, int *retErrno);

            /**
             * @brief 移除缓冲区中某一位置之前的数据。
             *
             * @param end 该位置。
             */
            void RetrieveUntil(const char *end)
            {
                assert(Peek() <= end);
                assert(end <= BeginWrite());
                Retrieve(end - Peek());
            }

            /**
             * @brief 查找缓冲区中CRLF的位置。
             *
             * @return const char*
             */
            const char *FindCRLF() const
            {
                const char *crlf = std::search(Peek(), BeginWrite(), CRLF, CRLF + 2);
                return crlf == BeginWrite() ? NULL : crlf;
            }

            /**
             * @brief 确保缓冲区有足够的空间写入数据。
             *
             * @param len
             */
            void EnsureWritableBytes(size_t len);

            /**
             * @brief 当新数据写入缓冲区时，向前移动写指针。
             *
             * @param len
             */
            void HasWritten(size_t len)
            {
                assert(len <= WritableBytes());
                tail_ += len;
            }

            /**
             * @brief 向后移动写指针以移除缓冲区末尾的数据。
             *
             * @param offset
             */
            void Unwrite(size_t offset)
            {
                assert(ReadableBytes() >= offset);
                tail_ -= offset;
            }

            /**
             * @brief 访问缓冲区中的一个字节。
             *
             * @param offset
             * @return const char&
             */
            const char &operator[](size_t offset) const
            {
                assert(ReadableBytes() >= offset);
                return Peek()[offset];
            }
            char &operator[](size_t offset)
            {
                assert(ReadableBytes() >= offset);
                return begin()[head_ + offset];
            }

          private:
            size_t head_; // 缓冲区读取位置的索引
            size_t initCap_; // 缓冲区的初始容量
            std::vector<char> buffer_; // 存储数据的字符向量
            size_t tail_; // 缓冲区写入位置的索引
            const char *begin() const
            {
                return &buffer_[0];
            }
            char *begin()
            {
                return &buffer_[0];
            }
        };

        inline void swap(MsgBuffer &one, MsgBuffer &two) noexcept
        {
            one.Swap(two);
        }
        inline uint64_t hton64(uint64_t n)
        {
            static const int one = 1;
            static const char sig = *(char *)&one;
            if (sig == 0)
                return n; // for big endian machine just return the input
            char *ptr = reinterpret_cast<char *>(&n);
            std::reverse(ptr, ptr + sizeof(uint64_t));
            return n;
        }
        inline uint64_t ntoh64(uint64_t n)
        {
            return hton64(n);
        }
    } // namespace network

} // namespace tmms
namespace std
{
    template <>
    inline void swap(tmms::network::MsgBuffer &one, tmms::network::MsgBuffer &two) noexcept
    {
        one.Swap(two);
    }
} // namespace std