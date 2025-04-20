cmake_minimum_required(VERSION 3.10)
project(LiveStreaming)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)



set(SOURCE
    ${CMAKE_SOURCE_DIR}/src/main.cpp
)

add_subdirectory(${CMAKE_SOURCE_DIR}/src/base)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/network)
add_subdirectory(${CMAKE_SOURCE_DIR}/tests)


add_executable(LiveStreaming ${SOURCE})


# 确保主程序链接了base库和jsoncpp库
target_link_libraries(LiveStreaming PRIVATE 
    base
    network
)
