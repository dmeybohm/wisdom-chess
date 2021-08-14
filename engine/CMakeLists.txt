set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake.modules/")

cmake_minimum_required(VERSION 3.10)

include_directories(. src)

# set the project name
project(Wisdom C CXX)

find_package(Threads REQUIRED)
find_package(doctest REQUIRED)
find_package(SQLite3 REQUIRED)

add_subdirectory(test)
add_subdirectory(src)