set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.20)

option(WISDOM_ASAN "Enable memory debugging of chess engine")

project(WisdomEngine CXX)

find_package(doctest)
find_package(benchmark)

if (!WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-stack-protector")
endif()

if (WISDOM_ASAN_BUILD)
    if (WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=address")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    endif()
endif()

if (doctest_FOUND)
    add_subdirectory(test)
else()
    message("*** Tests disabled (doctest not found)")
endif()

add_subdirectory(src)

if (benchmark_FOUND)
    add_subdirectory(benchmark)
else()
    message("*** Benchmarking disabled (Google benchmark not found)")
endif()

