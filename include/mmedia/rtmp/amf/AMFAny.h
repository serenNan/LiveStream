#pragma once
#include <cstdint>
#include <memory>
#include <string>

namespace tmms
{
    namespace mm
    {
        /**
         * @enum AMFDataType
         * @brief AMF数据类型枚举，定义了AMF0支持的所有数据类型
         *
         * AMF (Action Message Format) 是Flash和服务器之间通信的二进制格式，
         * 用于序列化ActionScript对象。此枚举定义了AMF0版本支持的数据类型。
         */
        enum AMFDataType
        {
            kAMFNumber = 0,    ///< 数字类型 (双精度浮点数)
            kAMFBoolean,       ///< 布尔类型 (true/false)
            kAMFString,        ///< 字符串类型 (UTF-8编码)
            kAMFObject,        ///< 对象类型 (键值对集合)
            kAMFMovieClip,     ///< 影片剪辑，保留类型，AMF0未使用
            kAMFNull,          ///< 空类型
            kAMFUndefined,     ///< 未定义类型
            kAMFReference,     ///< 引用类型 (引用之前出现过的对象)
            kAMFEcmaArray,     ///< ECMAScript数组 (关联数组)
            kAMFObjectEnd,     ///< 对象结束标志
            kAMStrictArray,    ///< 严格数组 (索引数组)
            kAMFDate,          ///< 日期类型 (UTC时间和时区偏移)
            kAMFLongString,    ///< 长字符串类型 (长度大于65535的字符串)
            kAMFUnsupported,   ///< 不支持的数据类型
            kAMFRecordset,     ///< 记录集，保留类型，AMF0未使用
            kAMFXMLDoc,        ///< XML文档类型
            kAMFTypedObject,   ///< 类型化对象 (带有类名的对象)
            kAMFAvmplus,       ///< AvmPlus对象，用于切换到AMF3格式
            kAMFInvalid = 0xff ///< 无效的AMF数据类型，使用0xff表示无效值
        };

        // 前向声明，防止循环引用
        class AMFObject;

        /// @typedef AMFObjectPtr 为AMFObject类定义的智能指针类型，使用std::shared_ptr管理内存
        using AMFObjectPtr = std::shared_ptr<AMFObject>;

        /**
         * @class AMFAny
         * @brief AMF数据类型的基类，所有具体AMF类型都继承自此类
         *
         * AMFAny类是所有AMF数据类型的抽象基类，提供了通用的接口和功能。
         * 继承了std::enable_shared_from_this，使对象能够安全地生成自身的智能指针。
         */
        class AMFAny : public std::enable_shared_from_this<AMFAny>
        {
          public:
            /**
             * @brief 构造函数，初始化带名称的AMF数据
             * @param name AMF数据的名称
             */
            AMFAny(const std::string &name);

            /**
             * @brief 默认构造函数，创建一个无名称的AMF数据
             */
            AMFAny();

            /**
             * @brief 解码AMF数据
             * @param data 要解码的二进制数据
             * @param size 数据大小
             * @param has 是否已包含类型标记，默认为false
             * @return 成功解码的字节数
             *
             * 纯虚函数，子类必须实现此函数来解析特定类型的AMF数据
             */
            virtual int Decode(const char *data, int size, bool has = false) = 0;

            /**
             * @brief 获取字符串值
             * @return 字符串值的常量引用
             *
             * 默认实现返回空字符串，字符串类型的子类会重写此方法
             */
            virtual const std::string &String();

            /**
             * @brief 获取布尔值
             * @return 布尔值
             *
             * 默认实现返回false，布尔类型的子类会重写此方法
             */
            virtual bool Boolean();

            /**
             * @brief 获取数字值
             * @return 双精度浮点数值
             *
             * 默认实现返回0.0，数字类型的子类会重写此方法
             */
            virtual double Number();

            /**
             * @brief 获取日期值
             * @return 表示日期的双精度浮点数（UTC时间戳，毫秒）
             *
             * 默认实现返回0.0，日期类型的子类会重写此方法
             */
            virtual double Date();

            /**
             * @brief 获取对象值
             * @return AMFObject类型的智能指针
             *
             * 默认实现返回nullptr，对象类型的子类会重写此方法
             */
            virtual AMFObjectPtr Object();

            /**
             * @brief 检查是否为字符串类型
             * @return 如果是字符串类型则返回true，否则返回false
             */
            virtual bool IsString();

            /**
             * @brief 检查是否为数字类型
             * @return 如果是数字类型则返回true，否则返回false
             */
            virtual bool IsNumber();

            /**
             * @brief 检查是否为布尔类型
             * @return 如果是布尔类型则返回true，否则返回false
             */
            virtual bool IsBoolean();

            /**
             * @brief 检查是否为日期类型
             * @return 如果是日期类型则返回true，否则返回false
             */
            virtual bool IsDate();

            /**
             * @brief 检查是否为对象类型
             * @return 如果是对象类型则返回true，否则返回false
             */
            virtual bool IsObject();

            /**
             * @brief 检查是否为空类型
             * @return 如果是空类型则返回true，否则返回false
             */
            virtual bool IsNull();

            /**
             * @brief 输出调试信息
             *
             * 纯虚函数，子类必须实现此函数来输出类型特定的调试信息
             */
            virtual void Dump() const = 0;

            /**
             * @brief 获取AMF数据的名称
             * @return 名称的常量引用
             */
            const std::string &Name() const;

            /**
             * @brief 获取子元素数量
             * @return 子元素数量
             *
             * 默认实现返回1，复合类型的子类（如对象、数组等）会重写此方法
             */
            virtual int32_t Count() const;

            /**
             * @brief 将双精度浮点数编码为AMF数字格式
             * @param output 输出缓冲区
             * @param dVal 要编码的双精度浮点数
             * @return 编码后的字节数
             */
            static int32_t EncodeNumber(char *output, double dVal);

            /**
             * @brief 将字符串编码为AMF字符串格式
             * @param output 输出缓冲区
             * @param str 要编码的字符串
             * @return 编码后的字节数
             */
            static int32_t EncodeString(char *output, const std::string &str);

            /**
             * @brief 将布尔值编码为AMF布尔格式
             * @param output 输出缓冲区
             * @param b 要编码的布尔值
             * @return 编码后的字节数
             */
            static int32_t EncodeBoolean(char *output, bool b);

            /**
             * @brief 将带名称的数字编码为AMF格式
             * @param output 输出缓冲区
             * @param name 属性名称
             * @param dVal 要编码的数字值
             * @return 编码后的字节数
             */
            static int32_t EncodeNamedNumber(char *output, const std::string &name, double dVal);

            /**
             * @brief 将带名称的字符串编码为AMF格式
             * @param output 输出缓冲区
             * @param name 属性名称
             * @param value 要编码的字符串值
             * @return 编码后的字节数
             */
            static int32_t EncodeNamedString(char *output, const std::string &name,
                                             const std::string &value);

            /**
             * @brief 将带名称的布尔值编码为AMF格式
             * @param output 输出缓冲区
             * @param name 属性名称
             * @param bVal 要编码的布尔值
             * @return 编码后的字节数
             */
            static int32_t EncodeNamedBoolean(char *output, const std::string &name, bool bVal);

            /**
             * @brief 虚析构函数
             *
             * 确保派生类的析构函数也被正确调用
             */
            virtual ~AMFAny();

          protected:
            /**
             * @brief 解码字符串数据
             * @param data 包含字符串的二进制数据
             * @return 解码后的字符串
             */
            static std::string DecodeString(const char *data);

            /**
             * @brief 将名称编码为AMF格式
             * @param buff 输出缓冲区
             * @param name 要编码的名称
             * @return 编码后的字节数
             */
            static int EncodeName(char *buff, const std::string &name);

            /**
             * @brief 将双精度浮点数直接写入缓冲区
             * @param buff 输出缓冲区
             * @param value 要写入的双精度浮点数
             * @return 写入的字节数
             */
            static int WriteNumber(char *buff, double value);

            std::string name_; ///< AMF数据的名称
        };
    } // namespace mm
} // namespace tmms
