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

### `<<` 运算符重载
对应代码：[LogStream.h](../include/base/LogStream.h#L33)
对应资料：[C++重载>>和<<（输入和输出运算符）详解](https://c.biancheng.net/view/2311.html)

### 链式引用
Learn
对应代码：[LogStream.h](../include/base/LogStream.h#L36)

### g_logger
对应代码：[LogStream.cpp](../src/base/LogStream.cpp#L9)

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

## 左值引用

在C++中，`std::shared_ptr<Event>& event` 这个声明包含几个关键部分，我来详细解释这个引用(&)的作用：

1. **std::shared_ptr<Event>**:
   - 这是一个智能指针模板，管理Event对象的生命周期
   - 使用引用计数机制自动释放内存

2. **& 符号的作用**:
   - 这里的`&`表示`event`是一个引用(左值引用)
   - 它直接引用`iter->second`这个shared_ptr对象本身，而不是创建副本
   - 这意味着对`event`的任何修改都会直接影响`iter->second`

3. **为什么在这里使用引用**:
   - 避免不必要的shared_ptr拷贝(会增加引用计数开销)
   - 可以直接修改map中的shared_ptr对象
   - 提高性能，减少内存操作

4. **与不使用引用的区别**:
```cpp
std::shared_ptr<Event> event = iter->second;  // 创建副本，增加引用计数
std::shared_ptr<Event>& event = iter->second; // 直接引用原对象
```

5. **使用场景**:
   - 当需要直接操作容器中的智能指针时
   - 当函数需要修改传入的shared_ptr时
   - 当需要避免智能指针的拷贝开销时

6. **注意事项**:
   - 引用必须确保被引用的对象生命周期足够长
   - 不能引用临时对象
   - 在异步编程中要特别注意线程安全问题

在你的代码中，这种写法可以高效地获取并可能修改events_ map中存储的Event对象。

## lamda表达式

对应代码：[EventLoopThread.cpp](../src/network/net/EventLoopThread.cpp)
```cpp
[this](){StartEventLoop();}
```

传一个`this`指针，然后在lamda表达式中调用私有成员函数`StartEventLoop()`