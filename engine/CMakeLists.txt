set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.10)

# set the project name
project(Wisdom CXX)

include_directories(. src)

set(CMAKE_MODULE_PATH, ${CMAKE_BINARY_DIR})
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(Threads REQUIRED)
find_package(doctest REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(gsl-lite REQUIRED)

add_subdirectory(test)
add_subdirectory(src)
