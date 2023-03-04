set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.20)

project(WisdomChess CXX)

# Make find_package() find dependencies in the binary dir:
if (ANDROID OR EMSCRIPTEN)
    list(APPEND CMAKE_FIND_ROOT_PATH "${CMAKE_BINARY_DIR}")
    list(APPEND CMAKE_FIND_ROOT_PATH "${CMAKE_BINARY_DIR}/conan-dependencies")
else()
    list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
endif()

option(WISDOM_CHESS_ASAN "Enable memory debugging of chess engine")
option(WISDOM_CHESS_FAST_TESTS "Enable building of fast tests" On)
option(WISDOM_CHESS_SLOW_TESTS "Enable building of slow tests")
option(WISDOM_CHESS_CONSOLE_UI "Enable building of console chess game" On)
option(WISDOM_CHESS_QML_UI "Enable building of QML UI" On)

if (QTDIR)
    list(APPEND CMAKE_PREFIX_PATH ${QTDIR})
    message("-- wisdom-chess: QTDIR added to CMAKE_PREFIX_PATH")
endif()

find_package(Qt6 6.4 QUIET COMPONENTS Quick)

if (WISDOM_CHESS_ASAN)
    message("-- wisdom-chess: Enabling address sanitizer")
    if (MSVC)
        add_compile_options(/fsanitize=address)
        add_link_options(/fsanitize=address /zi)
    else()
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()
endif()

add_subdirectory(engine)
add_subdirectory(ui)
