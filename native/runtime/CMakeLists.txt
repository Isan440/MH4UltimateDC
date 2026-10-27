cmake_minimum_required(VERSION 3.16)
project(mh4u_native_runtime LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(mh4u_native_runtime STATIC host_runtime.cpp host_runtime_callbacks.cpp code_module.cpp ../memory/guest_memory.cpp)
target_include_directories(mh4u_native_runtime PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(mh4u_native_runtime PUBLIC ${CMAKE_DL_LIBS})
target_compile_options(mh4u_native_runtime PRIVATE -Wall -Wextra -Wpedantic -ffp-contract=off)

# A tiny ABI-compatible shared library exercises the same dlopen/dlsym path used by
# generated game code, without requiring any copyrighted game assets.
add_library(mh4u_test_recompiled_code SHARED test_recompiled_module.c)
target_include_directories(mh4u_test_recompiled_code PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../tools/recomp/runtime")
set_target_properties(mh4u_test_recompiled_code PROPERTIES
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES)
set_source_files_properties(test_recompiled_module.c PROPERTIES
    COMPILE_OPTIONS "-fvisibility=default")

add_executable(mh4u_native_runtime_tests tests.cpp)
target_link_libraries(mh4u_native_runtime_tests PRIVATE mh4u_native_runtime)
target_compile_definitions(mh4u_native_runtime_tests PRIVATE
    MH4U_TEST_MODULE_PATH="$<TARGET_FILE:mh4u_test_recompiled_code>")
add_dependencies(mh4u_native_runtime_tests mh4u_test_recompiled_code)
# Release builds define NDEBUG; keep assert-based test checks active in CI.
target_compile_options(mh4u_native_runtime_tests PRIVATE -UNDEBUG)
enable_testing()
add_test(NAME native_runtime_basics COMMAND mh4u_native_runtime_tests)
