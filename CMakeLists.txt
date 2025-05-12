cmake_minimum_required(VERSION 3.10)
project(LiveStreaming)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

# 设置安装路径
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})



add_subdirectory(${CMAKE_SOURCE_DIR}/src/base)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/network)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/mmedia)
add_subdirectory(${CMAKE_SOURCE_DIR}/tests)
#add_subdirectory(${CMAKE_SOURCE_DIR}/note)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/live)
add_subdirectory(${CMAKE_SOURCE_DIR}/gui)


