set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.20)

project(WisdomChess CXX)

option(WISDOM_ASAN_BUILD "Enable memory debugging of chess engine")
find_package(Qt6 6.2 COMPONENTS Quick)

if (WISDOM_ASAN_BUILD)
    if (WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=address")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    endif()
endif()

add_subdirectory(engine)

if (Qt6_FOUND)
    add_subdirectory(qt-qml-gui)
endif()
