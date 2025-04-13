cmake_minimum_required(VERSION 3.10)

link_directories(${CMAKE_SOURCE_DIR}/third_party/gtest/lib)

set(TEST_SOURCE
    TestLogger.cpp
    TestSingleton.cpp
    TestStringUtils.cpp
    TestTask.cpp
    TestTTime.cpp
)

add_executable(LiveStreamingTest ${TEST_SOURCE})

target_include_directories(LiveStreamingTest PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(LiveStreamingTest
    libgtest.a
    libgtest_main.a
    base
)