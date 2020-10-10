set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake.modules/")
INCLUDE(CheckIncludeFiles)

cmake_minimum_required(VERSION 3.10)


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
        src/board.c src/board_hash.c src/check.c src/debug.c src/evaluate.c src/game.c src/generate.c src/material.c src/move.c
        src/move_list.c src/move_tree.c src/piece.c src/play.c src/search.c src/sha1.c src/str.c src/timer.c
        src/global.h src/board_check.c src/board_check.h  src/validate.c src/validate.h)

set_property(TARGET chess PROPERTY C_STANDARD 11)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

if (HAVE_WINSOCK2_H)
    TARGET_LINK_LIBRARIES(chess ws2_32)
endif (HAVE_WINSOCK2_H)

add_subdirectory(test)
