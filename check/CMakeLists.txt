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

# add the executable
add_executable(chess
    board.c board_hash.c check.c debug.c evaluate.c game.c generate.c material.c move.c
    move_list.c move_tree.c piece.c play.c search.c sha1.c str.c timer.c
)

#set_property(TARGET chess PROPERTY C_STANDARD 11)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

#
# Tests
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake.modules/")
enable_testing()
#find_path(CATCH_INCLUDE_DIR "catch.h")
#include_directories(${INCLUDE_DIRECTORIES} ${CATCH_INCLUDE_DIR})
add_executable(chess_tests
    test/test_main.cpp
    test/board_hash_test.cpp
    board.c board_hash.c check.c debug.c evaluate.c generate.c material.c move.c
    move_list.c move_tree.c piece.c search.c sha1.c str.c timer.c
)

if (HAVE_WINSOCK2_H)
    TARGET_LINK_LIBRARIES(chess ws2_32)
    TARGET_LINK_LIBRARIES(chess_tests ws2_32)
endif (HAVE_WINSOCK2_H)

INCLUDE(ParseAndAddCatchTests)
ParseAndAddCatchTests(chess_tests)
