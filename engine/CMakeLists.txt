set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.20)

option(WISDOM_ASAN_BUILD "Enable memory debugging of chess engine")

project(Wisdom CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-stack-protector")

if (WISDOM_ASAN_BUILD)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif()

add_subdirectory(test)
add_subdirectory(src)
add_subdirectory(benchmark)
