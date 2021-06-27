set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake.modules/")

INCLUDE(CheckIncludeFiles)
INCLUDE(CMakeHelpers.cmake)

cmake_minimum_required(VERSION 3.10)

include_directories(. src lib)

# set the project name
project(Wisdom C CXX)

find_package (Threads REQUIRED)
find_package (doctest REQUIRED)

find_package (SQLite3)
find_package(unofficial-sqlite3 CONFIG)

# add the executable
add_library(chess_lib OBJECT
        src/board.cpp src/check.cpp src/evaluate.cpp src/game.cpp src/generate.cpp src/material.cpp src/move.cpp
        src/move_tree.cpp src/piece.cpp src/search.cpp src/str.cpp src/move_timer.cpp
        src/global.hpp src/validate.cpp src/validate.hpp src/position.cpp src/position.hpp
        src/fen_parser.cpp src/fen_parser.hpp src/coord.cpp src/board_builder.cpp src/move_list.cpp src/move_list.hpp src/move_history.hpp
        src/move_history.cpp src/multithread_search.cpp src/multithread_search.hpp src/coord_iterator.cpp src/coord_iterator.hpp
        src/board_code.cpp src/board_code.hpp src/logger.cpp src/logger.hpp src/board_history.cpp src/board_history.hpp
        src/history.cpp src/transposition_table.cpp src/transposition_table.hpp src/variation_glimpse.cpp src/variation_glimpse.hpp
        src/output_format.cpp src/output_format.hpp src/analytics.cpp src/analytics.hpp src/search_result.cpp src/search_result.hpp
        src/uuid.cpp src/uuid.hpp)
add_executable(chess
        src/play.cpp
        src/analytics_sqlite.cpp src/analytics_sqlite.hpp
        $<TARGET_OBJECTS:chess_lib>)

target_precompile_headers(chess_lib PUBLIC PUBLIC src/global.hpp)
target_precompile_headers(chess REUSE_FROM chess_lib)

if (SQLite3_FOUND)
    target_link_libraries (chess_lib PRIVATE SQLite::SQLite3)
    target_link_libraries (chess PRIVATE SQLite::SQLite3)
elseif(unofficial-sqlite3_FOUND)
    target_link_libraries(chess_lib PRIVATE unofficial::sqlite3::sqlite3)
    target_link_libraries(chess PRIVATE unofficial::sqlite3::sqlite3)
    target_link_options(chess -ldl)
else()
    message( FATAL_ERROR "SQLite3 is required, and implementation was not found.")
endif()


add_subdirectory(test)
