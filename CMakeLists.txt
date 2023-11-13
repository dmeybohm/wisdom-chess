set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.20)

include(cmake/CPM.cmake)

project(WisdomChess CXX)

option(WISDOM_CHESS_ASAN "Enable memory debugging of chess engine")
option(WISDOM_CHESS_FAST_TESTS "Enable building of fast tests" On)
option(WISDOM_CHESS_SLOW_TESTS "Enable building of slow tests")
option(WISDOM_CHESS_CONSOLE_UI "Enable building of console chess game" On)
option(WISDOM_CHESS_QML_UI "Enable building of QML UI" On)
option(WISDOM_CHESS_REACT_UI "Enable building of React UI" On)
option(WISDOM_CHESS_PCH_ENABLED "Enable Pre-compiled headers" On)

if (QTDIR)
    list(APPEND CMAKE_PREFIX_PATH ${QTDIR})
    message("-- wisdom-chess: QTDIR added to CMAKE_PREFIX_PATH")
endif()

find_package(Qt6 QUIET COMPONENTS Quick)

if (WISDOM_CHESS_ASAN)
    message("-- wisdom-chess: Enabling address sanitizer")
    if (MSVC)
        add_compile_options(/fsanitize=address)
        add_link_options(/fsanitize=address /zi)
    else()
        add_compile_options(-fsanitize=address,undefined)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

add_subdirectory(engine)
add_subdirectory(ui)
