cmake_minimum_required(VERSION 3.10)
project(LiveStreaming)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

# 设置安装路径
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})


set(SOURCE
    ${CMAKE_SOURCE_DIR}/src/main.cpp
)

add_subdirectory(${CMAKE_SOURCE_DIR}/src/base)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/network)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/mmedia)
#add_subdirectory(${CMAKE_SOURCE_DIR}/tests)
#add_subdirectory(${CMAKE_SOURCE_DIR}/note)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/live)


add_executable(LiveStreaming ${SOURCE})


# 确保主程序链接了base库和jsoncpp库
target_link_libraries(LiveStreaming PRIVATE 
    base
    network
    mmedia
    live
)

# 安装目标
install(TARGETS LiveStreaming
        RUNTIME DESTINATION bin/sbin)

# 创建bin/sbin目录
install(CODE "file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/sbin)")

# 复制配置文件(如果有)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/conf/ 
        DESTINATION conf 
        OPTIONAL
        FILES_MATCHING PATTERN "*.json" PATTERN "*.conf")
