cmake_minimum_required(VERSION 3.10)

set(TEST_SOURCE
    TTime_test.cpp
    # test.cpp
    ${CMAKE_SOURCE_DIR}/src/base/TTime.cpp
)

add_executable(LiveStreamingTest ${TEST_SOURCE})

target_include_directories(LiveStreamingTest PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/base
)

target_link_libraries(LiveStreamingTest
    libgtest.a
    libgtest_main.a
)