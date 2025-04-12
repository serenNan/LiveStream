#include "base/Singleton.h"
#include "base/NonCopyable.h"
#include "gtest/gtest.h"
#include <iostream>

using namespace tmms::base;

class A : public NonCopyable
{
  public:
    A() = default;
    ~A() = default;
    void print()
    {
        std::cout << "A::print()" << std::endl;
      }
};

#define sA Singleton<A>::Instance()

TEST(SingletonTest, BasicTest)
{
    sA->print();
}


