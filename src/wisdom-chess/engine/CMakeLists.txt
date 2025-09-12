set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.20)

project(WisdomEngine CXX)

find_package(Threads REQUIRED)

CPMAddPackage("gh:microsoft/GSL@4.0.0")
CPMAddPackage(
  NAME doctest
  GITHUB_REPOSITORY doctest/doctest
  GIT_TAG v2.4.12
  OPTIONS
     "DOCTEST_NO_INSTALL On"
)

find_package(doctest)

if (ANDROID OR NOT WISDOM_CHESS_PCH_ENABLED)
    # Disable precompile headers because there is a confict on android where it links
    # some code as PIC and some as non-PIC and can't combine them.
    set(PCH_ENABLED OFF)
    message("-- wisdom-chess: precompiled headers disabled")
else()
    set(PCH_ENABLED ON)
    message("-- wisdom-chess: precompiled headers enabled")
endif()

add_library(wisdom-chess-core STATIC
        board_builder.hpp
        board_code.hpp
        board.hpp
        castling.hpp
        coord.hpp
        evaluate.hpp
        fen_parser.hpp
        game.hpp
        game_status.hpp
        generate.hpp
        global.hpp
        history.hpp
        logger.hpp
        material.hpp
        move.hpp
        move_list.hpp
        move_timer.hpp
        output_format.hpp
        piece.hpp
        position.hpp
        random.hpp
        search.hpp
        str.hpp
        threats.hpp
        board.cpp
        board_code.cpp
        coord.cpp
        evaluate.cpp
        fen_parser.cpp
        game.cpp
        game_status.cpp
        generate.cpp
        history.cpp
        logger.cpp 
        material.cpp 
        move.cpp
        move_list.cpp
        move_timer.cpp 
        output_format.cpp
        piece.cpp 
        position.cpp 
        search.cpp
        str.cpp)

if (PCH_ENABLED)
    target_precompile_headers(wisdom-chess-core PRIVATE PRIVATE global.hpp)
else()
    target_sources(wisdom-chess-core PRIVATE global.hpp)
endif()

target_include_directories(wisdom-chess-core PUBLIC ../..)
add_library(wisdom::chess ALIAS wisdom-chess-core)

target_link_libraries(wisdom-chess-core PUBLIC Microsoft.GSL::GSL)
target_link_libraries(wisdom-chess-core PUBLIC Threads::Threads)

if (EMSCRIPTEN)
    target_compile_options(wisdom-chess-core PUBLIC -pthread)
endif()

if (NOT WIN32 AND NOT EMSCRIPTEN)
    message("-- wisdom-chess: stack protector disabled for performance")
    target_compile_options(wisdom-chess-core PUBLIC -fno-stack-protector)
endif()

if (doctest_FOUND)
    message("-- wisdom-chess: Including tests with doctest")
    add_subdirectory(test)
else()
    message("-- wisdom-chess: Tests disabled (doctest not found)")
    set(WISDOM_CHESS_SLOW_TESTS Off)
    set(WISDOM_CHESS_FAST_TESTS Off)
endif()
