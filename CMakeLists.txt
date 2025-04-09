cmake_minimum_required(VERSION 3.10)
project(LiveStreaming)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find GoogleTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(SOURCE
    ${CMAKE_SOURCE_DIR}/src/base/TTime.cpp
)

set(INCLUDE
    ${CMAKE_SOURCE_DIR}/include/TTime.h
)

set(TEST
    ${CMAKE_SOURCE_DIR}/test/TTime_test.cpp
)

add_executable(LiveStreaming ${SOURCE} ${INCLUDE})

add_executable(TTime_test ${TEST} ${SOURCE})
target_include_directories(TTime_test PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link GoogleTest to test executable
target_link_libraries(TTime_test ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)

target_include_directories(${PROJECT_NAME} PRIVATE
${CMAKE_SOURCE_DIR}/include
)

# Enable testing
enable_testing()
add_test(NAME TTime_test COMMAND TTime_test)