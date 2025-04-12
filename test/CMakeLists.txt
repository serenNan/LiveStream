cmake_minimum_required(VERSION 3.10)

set(TEST_SOURCE
    TTimeTest.cpp
    StringUtilsTest.cpp
    SingletonTest.cpp
    TaskManagerTest.cpp
    LoggerTest.cpp
    ${CMAKE_SOURCE_DIR}/src/base/TTime.cpp
    ${CMAKE_SOURCE_DIR}/src/base/StringUtils.cpp
    ${CMAKE_SOURCE_DIR}/src/base/Task.cpp
    ${CMAKE_SOURCE_DIR}/src/base/TaskManager.cpp
    ${CMAKE_SOURCE_DIR}/src/base/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/base/LogStream.cpp
)

add_executable(LiveStreamingTest ${TEST_SOURCE})

target_include_directories(LiveStreamingTest PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(LiveStreamingTest
    libgtest.a
    libgtest_main.a
)