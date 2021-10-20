set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.20)

# set the project name
project(Wisdom CXX)

include_directories(src)

add_subdirectory(test)
add_subdirectory(src)
