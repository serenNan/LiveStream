#include "NonCopyable.h"
#include "Singleton.h"
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

int main()
{
    sA->print();
    return 0;
}


