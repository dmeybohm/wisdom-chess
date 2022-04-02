set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.20)

project(Wisdom CXX)

find_package(doctest REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-stack-protector")

if (doctest_FOUND)
    add_subdirectory(test)
endif()

add_subdirectory(src)
add_subdirectory(benchmark)
