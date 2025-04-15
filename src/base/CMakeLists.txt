# Base目录的CMake配置
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} BASE_SOURCE)

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