cmake_minimum_required(VERSION 3.5) # CMake install : https://cmake.org/download/
project(MainWindow LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "/usr/local/Qt/Qt6.5.3/6.5.3/gcc_64") # Qt Kit Dir
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 COMPONENTS Widgets REQUIRED) # Qt COMPONENTS
aux_source_directory(./src srcs)

# 添加资源文件
set(RESOURCE_FILES
    src/resources.qrc
)

# Specify MSVC UTF-8 encoding   
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_executable(${PROJECT_NAME}
    WIN32 # If you need a terminal for debug, please comment this statement 
    ${srcs}
    ${RESOURCE_FILES}
) 
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt6::Widgets
    base
    live
    mmedia
    network
) # Qt6 Shared Library