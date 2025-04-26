cmake_minimum_required(VERSION 3.10)

link_directories(${CMAKE_SOURCE_DIR}/third_party/gtest/lib)


# Base 库测试
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} TEST_SOURCE)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/base TEST_SOURCE)
add_executable(BaseTest ${TEST_SOURCE})

target_include_directories(BaseTest PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/gtest/include
)

target_link_libraries(BaseTest
    base
    network
    libgtest.a
    libgtest_main.a
)

# Network 库测试
add_executable(TestTcpConnection ./network/TestTcpConnection.cpp)
target_link_libraries(TestTcpConnection
    base
    network
)

add_executable(TestTcpServer ./network/TestTcpServer.cpp)
target_link_libraries(TestTcpServer
    base
    network
)

add_executable(TestTcpClient ./network/TestTcpClient.cpp)
target_link_libraries(TestTcpClient
    base
    network
)

add_executable(TestUdpClient ./network/TestUdpClient.cpp)
target_link_libraries(TestUdpClient
    base
    network
)

add_executable(TestUdpServer ./network/TestUdpServer.cpp)
target_link_libraries(TestUdpServer
    base
    network
)
  
# Mmedia 库测试
add_executable(TestHandShakeClient ./rtmp/TestHandShakeClient.cpp)
target_link_libraries(TestHandShakeClient
    base
    network
    mmedia
)

add_executable(TestHandShakeServer ./rtmp/TestHandShakeServer.cpp)
target_link_libraries(TestHandShakeServer
    base
    network
    mmedia
)