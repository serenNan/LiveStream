# Base目录的CMake配置
set(NETWORK_SOURCE
    net/EventLoop.cpp
)

add_library(network ${NETWORK_SOURCE})

target_include_directories(network PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include/network
    ${CMAKE_SOURCE_DIR}/include/network/net
    ${CMAKE_SOURCE_DIR}/include/network/base
)

target_link_libraries(network PUBLIC
    base
)