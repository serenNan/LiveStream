#pragma once
#include "AMFAny.h"
#include <vector>

namespace tmms
{
    namespace mm
    {
        /**
         * @typedef AMFAnyPtr
         * @brief AMFAny类的智能指针类型
         *
         * 使用std::shared_ptr管理AMFAny对象的内存，确保安全的内存管理和资源释放
         */
        using AMFAnyPtr = std::shared_ptr<AMFAny>;

        /**
         * @class AMFObject
         * @brief 表示AMF数据中的对象类型
         *
         * AMFObject类用于表示AMF数据中的对象，它是一个包含多个命名属性的复合类型。
         * 每个属性都是AMFAny的子类实例，可以是任意类型（数字、字符串、布尔值、对象等）。
         * 在RTMP通信中常用于传输结构化数据，如媒体元数据、命令参数等。
         * 继承自AMFAny基类，实现了特定于对象类型的序列化和反序列化功能。
         */
        class AMFObject : public AMFAny
        {
          public:
            /**
             * @brief 构造函数，初始化带名称的AMFObject对象
             * @param name AMF数据的名称
             */
            AMFObject(const std::string &name);

            /**
             * @brief 默认构造函数，创建一个无名称的AMFObject对象
             */
            AMFObject();

            /**
             * @brief 解码AMF对象类型数据
             * @param data 要解码的二进制数据
             * @param size 数据大小
             * @param has 是否已包含类型标记，默认为false
             * @return 成功解码的字节数
             *
             * 从给定的数据缓冲区解析AMF对象，包括所有属性及其值
             */
            int Decode(const char *data, int size, bool has = false) override;

            /**
             * @brief 检查是否为对象类型
             * @return 总是返回true，表示这是一个对象类型
             */
            bool IsObject() override;

            /**
             * @brief 获取对象值
             * @return 当前对象的智能指针
             */
            AMFObjectPtr Object() override;

            /**
             * @brief 输出调试信息
             *
             * 输出该AMFObject对象的详细信息，包括名称和所有属性
             */
            void Dump() const override;

            /**
             * @brief 单次解码AMF属性
             * @param data 要解码的二进制数据
             * @param size 数据大小
             * @param has 是否已包含类型标记，默认为false
             * @return 成功解码的字节数
             *
             * 从给定的数据缓冲区解析单个AMF属性，并添加到对象的属性列表中
             */
            int DecodeOnce(const char *data, int size, bool has = false);

            /**
             * @brief 通过名称获取对象属性
             * @param name 属性名称
             * @return 属性的智能指针的常量引用
             *
             * 返回指定名称的属性，如果找不到则返回一个空指针
             */
            const AMFAnyPtr &Property(const std::string &name) const;

            /**
             * @brief 通过索引获取对象属性
             * @param index 属性索引
             * @return 属性的智能指针的常量引用
             *
             * 返回指定索引的属性，索引从0开始
             */
            const AMFAnyPtr &Property(int index) const;

            /**
             * @brief 析构函数
             */
            ~AMFObject();

          private:
            std::vector<AMFAnyPtr>
                properties_; ///< 存储对象的所有属性，每个属性都是AMFAny的智能指针
        };
    } // namespace mm
} // namespace tmms