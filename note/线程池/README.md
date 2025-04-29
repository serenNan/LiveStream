# 线程池测试程序

这个目录包含用于测试和理解项目线程池实现的示例代码和文档。

## 文件说明

- `simple_thread_pool_test.cpp` - 简单的线程池测试程序
- `线程池实现分析.md` - 对线程池实现的详细分析
- `示例代码.cpp` - 可以添加更多测试代码
- `CMakeLists.txt` - 构建配置文件

## 编译和运行

### 编译方法

```bash
# 进入线程池目录
cd note/线程池

# 创建构建目录
mkdir build
cd build

# 配置CMake
cmake ..

# 编译
make
```

### 运行测试程序

```bash
# 运行简单测试
./bin/simple_thread_pool_test
```

## 注意事项

1. 确保项目的 `include` 和 `lib` 目录已正确设置
2. 如果你的项目库路径不同，请修改 `CMakeLists.txt` 中的 `PROJECT_ROOT_DIR` 和库文件路径
3. 默认使用 C++17 标准，如需调整请修改 `CMAKE_CXX_STANDARD` 变量

## 如何添加更多测试

1. 创建新的 `.cpp` 文件
2. 在 `CMakeLists.txt` 中添加新的 `add_executable` 和 `target_link_libraries` 指令
3. 重新运行CMake配置和构建命令 