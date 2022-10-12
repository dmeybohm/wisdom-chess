set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.20)

project(WisdomChess CXX)

option(WISDOM_CHESS_ASAN "Enable memory debugging of chess engine")
option(WISDOM_CHESS_FAST_TESTS "Enable building of fast tests" On)
option(WISDOM_CHESS_SLOW_TESTS "Enable building of slow tests")

if (QTDIR)
    list(APPEND CMAKE_PREFIX_PATH ${QTDIR})
    message("-- wisdom-chess: QTDIR added to CMAKE_PREFIX_PATH")
endif()

find_package(Qt6 6.4 QUIET COMPONENTS Quick)

if (WISDOM_CHESS_ASAN)
    message("-- wisdom-chess: Enabling address sanitizer")
    if (WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=address")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    endif()
endif()

if (Qt6_FOUND)
    message("-- wisdom-chess: Qt6 GUI enabled.")
    add_subdirectory(qt-qml-gui)
else()
    message("-- wisdom-chess: Qt6 GUI disabled")
endif()

add_subdirectory(engine)
