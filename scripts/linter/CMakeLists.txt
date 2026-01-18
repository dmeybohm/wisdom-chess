cmake_minimum_required(VERSION 3.20)
project(wisdom-linter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(wisdom-linter
    main.cpp
    linter.cpp
    linter.hpp
    rules/init.cpp
    rules/namespace_braces.cpp
    rules/trailing_return_type.cpp
    rules/test_macro_spacing.cpp
    rules/function_call_spacing.cpp
)

target_compile_options(wisdom-linter PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
