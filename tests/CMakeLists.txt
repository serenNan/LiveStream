cmake_minimum_required(VERSION 3.10)

link_directories(${CMAKE_SOURCE_DIR}/third_party/gtest/lib)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} TEST_SOURCE)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/network TEST_SOURCE)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/base TEST_SOURCE)

# add_executable(LiveStreamingTest ${TEST_SOURCE})

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


# target_include_directories(LiveStreamingTest PRIVATE
#     ${CMAKE_SOURCE_DIR}/third_party/gtest/include
# )

# target_link_libraries(LiveStreamingTest
#     base
#     network
#     libgtest.a
#     libgtest_main.a
# )