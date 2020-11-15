set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake.modules/")
INCLUDE(CheckIncludeFiles)

cmake_minimum_required(VERSION 3.10)
include_directories(. src)

set(CMAKE_BUILD_TYPE Debug)

# set the project name
project(WizDumb)

CHECK_INCLUDE_FILES("netinet/in.h" HAVE_NETINET_H)
CHECK_INCLUDE_FILES("winsock2.h" HAVE_WINSOCK2_H)
CHECK_INCLUDE_FILES("time.h" HAVE_TIME_H)
CHECK_INCLUDE_FILES("sys/time.h" HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILES("unistd.h" HAVE_UNISTD_H)

# add the executable
add_executable(chess
        src/board.cpp src/board_hash.cpp src/check.cpp src/debug.cpp src/evaluate.cpp src/game.cpp src/generate.cpp src/material.cpp src/move.cpp
        src/move_tree.cpp src/piece.cpp src/play.cpp src/search.cpp src/sha1.cpp src/str.cpp src/timer.cpp
        src/global.h src/board_check.cpp src/board_check.h src/validate.cpp src/validate.h src/position.cpp src/position.h src/realloc_vector.cpp src/realloc_vector.hpp
        src/fen.cpp src/fen.hpp src/coord.cpp src/board_builder.cpp src/move_list.cpp src/move_list.hpp src/move_history.hpp src/move_history.cpp)

set_property(TARGET chess PROPERTY C_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -fsanitize=address -Wall -Wno-unused-parameter -Wno-unused-function")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -O3 -fsanitize=address -march=native")

# Speed up linking on Linux
execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE ld_version)
if ("${ld_version}" MATCHES "GNU gold")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
endif()

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

if (HAVE_WINSOCK2_H)
    TARGET_LINK_LIBRARIES(chess ws2_32)
endif (HAVE_WINSOCK2_H)

add_subdirectory(test)
