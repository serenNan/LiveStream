cmake_minimum_required(VERSION 3.10)

set(TEST_SOURCE
    TestLogger.cpp
    TestSingleton.cpp
    TestStringUtils.cpp
    TestTask.cpp
    TestTTime.cpp
    ${CMAKE_SOURCE_DIR}/src/base/TTime.cpp
    ${CMAKE_SOURCE_DIR}/src/base/StringUtils.cpp
    ${CMAKE_SOURCE_DIR}/src/base/Task.cpp
    ${CMAKE_SOURCE_DIR}/src/base/TaskManager.cpp
    ${CMAKE_SOURCE_DIR}/src/base/Logger.cpp
    ${CMAKE_SOURCE_DIR}/src/base/LogStream.cpp
    ${CMAKE_SOURCE_DIR}/src/base/FileLog.cpp
    ${CMAKE_SOURCE_DIR}/src/base/FileLogManager.cpp

)

add_executable(LiveStreamingTest ${TEST_SOURCE})

target_include_directories(LiveStreamingTest PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(LiveStreamingTest
    libgtest.a
    libgtest_main.a
)