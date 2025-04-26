# Mmedia目录的CMake配置
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} MEDIA_SOURCE)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/base MEDIA_SOURCE)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/rtmp MEDIA_SOURCE)


add_library(mmedia ${MEDIA_SOURCE})

target_include_directories(mmedia PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include/mmedia
    ${CMAKE_SOURCE_DIR}/include/mmedia/base
    ${CMAKE_SOURCE_DIR}/include/mmedia/rtmp
)

target_link_libraries(mmedia PUBLIC
    base
    network
    crypto
)