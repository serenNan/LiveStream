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
详细资料：[菜鸟教程](https://www.runoob.com/cplusplus/cpp-libs-sstream.html)

### `<<` 运算符重载
对应文件：[LogStream.h](../include/base/LogStream.h#L33)
对应资料：[C++重载>>和<<（输入和输出运算符）详解](https://c.biancheng.net/view/2311.html)

### 链式引用
Learn
对应文件：[LogStream.h](../include/base/LogStream.h#L36)

### g_logger
对应文件：[LogStream.cpp](../src/base/LogStream.cpp#L9)

ai讲解：
`g_logger` 是一个全局日志记录器指针，它的主要作用和特点如下：

1. **核心功能**：
   - 作为整个日志系统的全局访问点
   - 存储当前使用的日志记录器实例
   - 通过指针实现运行时多态（可指向不同的Logger子类）

2. **设计特点**：
   - 使用 `extern` 声明（在头文件中）和实际定义（在cpp文件中）分离
   - 初始化为 `nullptr` 确保程序启动时明确状态
   - 通过指针而非实例实现灵活配置

3. **使用场景**：
   - 被所有日志宏（LOG_TRACE/DEBUG/INFO等）使用
   - 在日志流构造时作为参数传递
   - 最终在 `~LogStream()` 中调用其 `WriteLog()` 方法输出日志

4. **典型工作流程**：
```cpp
// 初始化阶段
g_logger = new FileLogger("app.log"); 

// 使用阶段（通过宏）
LOG_INFO << "Application started";

// 内部实际展开为：
LogStream(g_logger, __FILE__, __LINE__, kInfo) << "Application started";
```

5. **线程安全考虑**：
   - 需要外部确保初始化时的线程安全
   - 日志输出方法应自带线程安全机制

这个全局指针是连接日志前端（用户调用的LOG_XXX宏）和后端（实际写日志的实现）的关键桥梁。