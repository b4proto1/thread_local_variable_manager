cmake_minimum_required (VERSION 3.8)

project ("thread_local_variable_manager")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(Test/test_00_basic_usage)
add_subdirectory(Test/test_01_performance)
#add_subdirectory(Test/test_02_thread_safety)
