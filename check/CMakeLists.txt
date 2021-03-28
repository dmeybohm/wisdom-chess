set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake.modules/")
INCLUDE(CheckIncludeFiles)

cmake_minimum_required(VERSION 3.10)
include_directories(. src lib)

set(CMAKE_BUILD_TYPE Debug)

# set the project name
project(Wisdom)

# add the executable
add_library(chess_lib OBJECT
        src/board.cpp src/check.cpp src/evaluate.cpp src/game.cpp src/generate.cpp src/material.cpp src/move.cpp
        src/move_tree.cpp src/piece.cpp  src/search.cpp src/str.cpp src/move_timer.cpp
        src/global.hpp src/validate.cpp src/validate.hpp src/position.cpp src/position.hpp
        src/fen_parser.cpp src/fen_parser.hpp src/coord.cpp src/board_builder.cpp src/move_list.cpp src/move_list.hpp src/move_history.hpp
        src/move_history.cpp src/multithread_search.cpp src/multithread_search.hpp src/coord_iterator.cpp src/coord_iterator.hpp
        src/board_code.cpp src/board_code.hpp src/output.cpp src/output.hpp src/board_history.cpp src/board_history.hpp
        src/history.cpp src/transposition_table.cpp src/transposition_table.hpp src/variation_glimpse.cpp src/variation_glimpse.hpp)
add_executable(chess src/play.cpp $<TARGET_OBJECTS:chess_lib>)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using Clang
  set(CMAKE_CXX_FLAGS "-Wall -Wextra")
  set(CMAKE_CXX_FLAGS_DEBUG "-g")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3 -Wall -Wno-unused-parameter -Wno-unused-function")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall -Wextra")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")

  # Speed up linking on Linux
  execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE ld_version)
  if ("${ld_version}" MATCHES "GNU gold")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags -lpthread")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags -lpthread")
  endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  # using Intel C++
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++latest")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 /std:c++latest")
endif()

add_subdirectory(test)