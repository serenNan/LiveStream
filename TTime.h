#pragma once
#include <cstdint>
#include <string>

namespace tmms
{
namespace base
{
class TTime
{
  public:
    static int64_t NowMS();
    static int64_t Now();
    static int64_t Now(int &year, int &mouth, int &day, int &hour, int &minute, int &second);
    static std::string ISOTime();
};
} // namespace base
} // namespace tmms