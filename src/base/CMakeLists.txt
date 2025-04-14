# Base目录的CMake配置
set(BASE_SOURCE
    TTime.cpp
    StringUtils.cpp
    Task.cpp
    TaskManager.cpp
    Logger.cpp
    LogStream.cpp
    FileLog.cpp
    FileLogManager.cpp
    Config.cpp
    ConfigManager.cpp
)

add_library(base ${BASE_SOURCE})

target_include_directories(base PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/base
    ${CMAKE_SOURCE_DIR}/third_party/jsoncpp/include 
)

target_link_libraries(base PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/jsoncpp/lib/libjsoncpp.a
)