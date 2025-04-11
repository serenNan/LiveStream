# 基础函数库

这一章主要实现一些基础的函数，比如时间操作，文件操作，日记记录，定时任务等。
这些函数比较通用，很多模块都可能使用。

---

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

## 1. 时间工具类(TTime)

一些时间类

- `NowMS()` 获取当前UTC时间戳(毫秒)
- `Now()` 获取当前UTC时间戳(秒)
- `Now(int year, int mouth, int day, int hour, int minte, int second)` 获取时间戳并分解为年月日时分秒
- `ISOTime()` 获取ISO 8601格式时间字符串`

---

## 2. 字符串与文件工具类(StringUtils)

- `StartsWith()` 检查字符串是否以指定子串开头
- `EndsWith()` 检查字符串是否以指定子串结尾
- `FilePath()` 获取文件路径中的目录部分
- `FileNameExt()` 获取带扩展名的文件名
- `FileName()` 获取不带扩展名的文件名
- `Extension()` 获取文件扩展名
- `SplitString()` 按分隔符分割字符串

---

## 3. 单例模式实现

### 单例介绍

保证一个类仅有一个实例

- 只能自行创建实例
- 多线程初始化竞争
- 不可复制和移动

提供一个访问实例的全局访问点

### pthread_once 介绍

- 一次性初始化
  函数原型

```cpp
int pthread once(pthread once t*once control,void(*init routine)(void));
```

参数：

- `once control`控制变量
- `init routine`初始化函数返回值
  返回值：
- 若成功返回0，若失败返回错误编号

### NonCopyable 类介绍

C++的三/五法则：

- c++98
  - 拷贝构造函数
  - 拷贝赋值函数
  - 析构函数
- c++11
  - 移动构造函数
  - 移动赋值运算符

这五个相当于一个整体，定义了其中一个，系统编译器会认为其他几个也是需要的，系统会默认合成。

我们可以阻止默认合成

```cpp
class NonCopyable
{
    // 这个类是作为基类，所以定义成保护属性
  protected:
    NonCopyable(){};
    ~NonCopyable(){};
    // 删除拷贝构造函数
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable) =delete;
};
```

这样就能阻止拷贝

### 单例模式设计

```cpp
template <typename T> 
class Singleton : public NonCopyable {
    // ... 
};
```

- 继承 `NonCopyable` 确保不可赋值
- 模板类设计可复用于任意类型

**关键点实现**

1. 线程安全初始化

```cpp
static T *&Instance()
  {
    pthread_once(&ponce_, &Singleton::init);
    return value_;
  }
```

- 使用 `pthread_once`保证多线程环境下只初始化一次
- 返回引用避免指针被意外释放

2. 资源管理

```cpp
static void init()
{
    if (!value_)
    {
        value_ = new T();
    }
}
```

- 延迟初始化（首次访问时创建）

3. 静态成员定义

```cpp
template <typename T> pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;
```

- 作用 ：初始化POSIX线程的"一次性控制变量"
- 关键点 ：
  - `PTHREAD_ONCE_INIT` 是 `POSIX`标准定义的宏，确保 `ponce_` 处于初始未触发状态
  - 每个模板实例（每个单例类型）都有自己独立的 `ponce_` 变量
  - 保证首次调用 `pthread_once()` 时会执行初始化函数

```cpp
template <typename T> T *Singleton<T>::value_ = nullptr;
```

- 作用 ：初始化单例实例指针为空
- 关键点 ：
  - 使用 `nullptr` 明确表示初始状态
  - 延迟初始化设计：直到首次调用 `Instance()` 时才创建对象
  - 每个模板实例有自己独立的 `value_` 指针

## 4. 定时任务

### 定时任务 Task

**定时任务的特性**

- 在规定的时间执行
- 可以单次执行，也可以循环执行
- 通过回调函数执行任务

**定时任务的设计**

- 封装定时任务，包含任务回调函数和执行间隔时间
- 成员变量
  - `when` 时间点
  - `interval`间隔时间
  - `callback` 回调函数
- 成员函数
  - `Run()`: 执行任务
  - `Restart()`: 重新启动任务
  - `When()`: 获取下次执行时间

使用实例：

```cpp
// 单次任务（5秒后执行）
auto task1 = std::make_shared<Task>([](){
    std::cout << "执行一次" << std::endl;
}, TTime::NowMS() + 5000);

// 循环任务（立即开始，每秒执行）
auto task2 = std::make_shared<Task>([](){
    std::cout << "每秒执行" << std::endl; 
}, TTime::NowMS(), 1000);
```

### 定时任务管理器 TaskManager

**定时任务管理器的功能点**

- 管理所有定时任务，使用单例模式实现
- 成员变量
  - `tasks` 存放所有的定时任务，使用的是一个 `unordered_set`集合
  - `lock` 互斥锁
- 成员函数：
  - `OnWork()`: 执行所有到期任务
  - `Add()`: 添加任务
  - `Del()`: 删除任务

**定时任务器的算法**

- 直接遍历
- 最小时间堆
- 时间轮
  选择直接遍历，因为整个直播系统的全局定时任务数很少，而且每个任务的执行时间都比较短，所以直接遍历也可以。

解释一下 `OnWork`:

```cpp
void TaskManager::OnWork()
{
    // 1. 加锁保证线程安全
    std::lock_guard<std::mutex> lock(lock_);
  
    // 2. 获取当前时间戳(毫秒)
    int64_t now = TTime::NowMS();
  
    // 3. 遍历所有任务
    for (auto iter = tasks_.begin(); iter != tasks_.end();)
    {
        // 4. 检查任务是否到期(执行时间 <= 当前时间)
        if ((*iter)->When() < now)
        {
            // 5. 执行任务回调函数
            (*iter)->Run();
          
            // 6. 再次检查任务时间(判断是否是单次任务)
            if ((*iter)->When() < now)
            {
                // 7. 如果是单次任务，从列表中移除
                iter = tasks_.erase(iter);
                continue;
            }
        }
        // 8. 移动到下一个任务
        iter++;
    }
}
```

