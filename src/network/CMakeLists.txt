# Network目录的CMake配置
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} NETWORK_SOURCE)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/net NETWORK_SOURCE)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/base NETWORK_SOURCE)

# message(STATUS "NETWORK_SOURCE: ${NETWORK_SOURCE}")

add_library(network ${NETWORK_SOURCE})

target_include_directories(network PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include/network
    ${CMAKE_SOURCE_DIR}/include/network/net
    ${CMAKE_SOURCE_DIR}/include/network/base
)

target_link_libraries(network PUBLIC
    base
)