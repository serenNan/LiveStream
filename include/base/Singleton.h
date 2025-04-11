#pragma once
#include "NonCopyable.h"
#include <pthread.h>

namespace tmms
{
    namespace base
    {
        /**
         * @brief 单例模板类，提供线程安全的单例对象访问
         *
         * @tparam T 单例类的类型
         */
        template <typename T> class Singleton : public NonCopyable
        {
          public:
            Singleton() = delete;
            ~Singleton() = delete;

            /**
             * @brief 获取单例实例
             *
             * @return T*& 返回单例对象的引用
             */
            static T *&Instance()
            {
                pthread_once(&ponce_, &Singleton::init);
                return value_;
            }

          private:
            /**
             * @brief 初始化单例对象
             *
             * 线程安全地创建单例对象
             */
            static void init()
            {
                if (!value_)
                {
                    value_ = new T();
                }
            }

            static pthread_once_t ponce_;
            static T *value_;
        };
        template <typename T> pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;
        template <typename T> T *Singleton<T>::value_ = nullptr;

    } // namespace base
} // namespace tmms