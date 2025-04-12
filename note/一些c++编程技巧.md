# 编程技巧

这里主要是写一些c++编程的一些技巧，方便理解整个项目。

## 命名空间设计

为确保代码的模块化和避免符号冲突，采用两级命名空间结构：

```cpp
#pragma once
#include "NonCopyable.h"
#include <pthread.h>

// 项目根命名空间
namespace tmms {  // Transmedia Media Streaming 的缩写
    // 基础模块命名空间
    namespace base {
        // 基础功能实现...
    } // namespace base
} // namespace tmms
```

`tmms`是"Transmedia Media Streaming（跨媒体流处理）"的缩写，这个标识着整个项目。

`base`是本章要写的一些基本模板类。

## Log日志库

### C++ 标准库 <sstream>

对应文件：[LogStream.h](../include/base/LogStream.h#L40)
[菜鸟教程](https://www.runoob.com/cplusplus/cpp-libs-sstream.html)

### `<<` 运算符重载
对应文件：[LogStream.h](../include/base/LogStream.h#L33)
[C++重载>>和<<（输入和输出运算符）详解](https://c.biancheng.net/view/2311.html)



  