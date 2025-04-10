cmake_minimum_required(VERSION 3.10)
project(LiveStreaming)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 链接库文件目录
link_directories(${CMAKE_SOURCE_DIR}/lib)

set(SOURCE
    ${CMAKE_SOURCE_DIR}/src/main.cpp
)

add_subdirectory(${CMAKE_SOURCE_DIR}/src/base)
add_subdirectory(${CMAKE_SOURCE_DIR}/test)

add_executable(LiveStreaming ${SOURCE})

target_include_directories(${PROJECT_NAME} PRIVATE
${CMAKE_SOURCE_DIR}/include
${CMAKE_SOURCE_DIR}/include/base
)

# 链接库
target_link_libraries(${PROJECT_NAME}
    libgtest.a
    libgtest_main.a
)
