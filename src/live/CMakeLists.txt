# Live目录的CMake配置
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/base LIVE_SOURCE)

add_library(live ${LIVE_SOURCE})

target_include_directories(live PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/live
    ${CMAKE_SOURCE_DIR}/include/live/base
    ${CMAKE_SOURCE_DIR}/third_party/jsoncpp/include 
)

target_link_libraries(live PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/jsoncpp/lib/libjsoncpp.a
)