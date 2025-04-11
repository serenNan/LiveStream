#pragma once

namespace tmms
{
namespace base
{
/**
 * @brief 不可拷贝类，用于禁止派生类的拷贝构造和赋值操作
 * 
 * 通过继承此类可以防止派生类被拷贝构造或赋值
 */
class NonCopyable
{
  protected:
    /**
     * @brief 构造函数
     * 
     * 定义为protected以允许派生类继承
     */
    NonCopyable(){};
    
    /**
     * @brief 析构函数
     * 
     * 定义为protected以允许派生类继承
     */
    ~NonCopyable(){};
    
    /**
     * @brief 删除拷贝构造函数
     * 
     * 禁止拷贝构造
     */
    NonCopyable(const NonCopyable &) = delete;
    
    /**
     * @brief 删除赋值操作符
     * 
     * 禁止赋值操作
     */
    NonCopyable &operator=(const NonCopyable) = delete;
};
} // namespace base
} // namespace tmms