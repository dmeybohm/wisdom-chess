set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake.modules/")

INCLUDE(CheckIncludeFiles)
INCLUDE(CMakeHelpers.cmake)

cmake_minimum_required(VERSION 3.10)

include_directories(. src lib)

# set the project name
project(Wisdom C CXX)

find_package(Threads REQUIRED)

# add the executable

add_library(chess_lib OBJECT
        src/board.cpp src/check.cpp src/evaluate.cpp src/game.cpp src/generate.cpp src/material.cpp src/move.cpp
        src/move_tree.cpp src/piece.cpp  src/search.cpp src/str.cpp src/move_timer.cpp
        src/global.hpp src/validate.cpp src/validate.hpp src/position.cpp src/position.hpp
        src/fen_parser.cpp src/fen_parser.hpp src/coord.cpp src/board_builder.cpp src/move_list.cpp src/move_list.hpp src/move_history.hpp
        src/move_history.cpp src/multithread_search.cpp src/multithread_search.hpp src/coord_iterator.cpp src/coord_iterator.hpp
        src/board_code.cpp src/board_code.hpp src/logger.cpp src/logger.hpp src/board_history.cpp src/board_history.hpp
        src/history.cpp src/transposition_table.cpp src/transposition_table.hpp src/variation_glimpse.cpp src/variation_glimpse.hpp src/output_format.cpp src/output_format.hpp src/analytics.cpp src/analytics.hpp src/search_result.cpp src/search_result.hpp src/analytics_sqlite.cpp src/analytics_sqlite.hpp src/uuid.cpp src/uuid.hpp)
add_executable(chess src/play.cpp $<TARGET_OBJECTS:chess_lib>)
target_precompile_headers(chess_lib PUBLIC PUBLIC src/global.hpp)
target_precompile_headers(chess REUSE_FROM chess_lib)

if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  find_package (SQLite3)

  find_package (doctest REQUIRED)
  target_link_libraries (chess_lib PRIVATE ${SQLite3_LIBRARIES})
  target_link_libraries (chess PRIVATE ${SQLite3_LIBRARIES})
else()
  find_package(unofficial-sqlite3 CONFIG REQUIRED)
  find_package(doctest CONFIG REQUIRED)
  target_link_libraries(chess_lib PRIVATE unofficial::sqlite3::sqlite3)
  target_link_libraries(chess PRIVATE unofficial::sqlite3::sqlite3)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  # using Clang
  add_compile_options(-Wall -Wextra)

#  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1 -fno-omit-frame-pointer -fsanitize=address")
#  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread")

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3 -Wall -Wno-unused-parameter -Wno-unused-function")
  add_compile_options(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall -Wextra -fstrict-enums -DDOCTEST_CONFIG_SUPER_FAST_ASSERTS")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")

  # Using LLVM's linked
#  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fuse-ld=lld")
#  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fuse-ld=lld")
#  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -lpthread")
#  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lpthread")

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