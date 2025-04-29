add_executable(simple_thread_pool_test ./code/simple_thread_pool_test.cpp)
target_link_libraries(simple_thread_pool_test
    base
    network
)