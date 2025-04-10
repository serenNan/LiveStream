# Base目录的CMake配置
set(BASE_SOURCE
    TTime.cpp
)

add_library(base ${BASE_SOURCE})

target_include_directories(base PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/base

)