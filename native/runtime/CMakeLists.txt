cmake_minimum_required(VERSION 3.16)
project(mh4u_native_runtime LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(mh4u_native_runtime STATIC host_runtime.cpp host_runtime_callbacks.cpp)
target_include_directories(mh4u_native_runtime PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(mh4u_native_runtime PRIVATE -Wall -Wextra -Wpedantic -ffp-contract=off)

add_executable(mh4u_native_runtime_tests tests.cpp)
target_link_libraries(mh4u_native_runtime_tests PRIVATE mh4u_native_runtime)
enable_testing()
add_test(NAME native_runtime_basics COMMAND mh4u_native_runtime_tests)
