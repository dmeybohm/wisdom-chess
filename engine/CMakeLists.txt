set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.20)

project(WisdomEngine CXX)

find_package(doctest)
find_package(benchmark)

if (!WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-stack-protector")
endif()

if (doctest_FOUND)
    add_subdirectory(test)
else()
    message("*** Tests disabled (doctest not found)")
endif()

add_subdirectory(src)

if (benchmark_FOUND AND WISDOM_CHECK_BENCHMARK)
    add_subdirectory(benchmark)
else()
    message("*** Benchmarking disabled (Google benchmark not found)")
endif()

