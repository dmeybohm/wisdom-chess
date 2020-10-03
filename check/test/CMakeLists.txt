#
# Tests
#

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake.modules/")

add_executable(chess_tests
        ../src/board.c ../src/board_hash.c ../src/check.c ../src/debug.c ../src/evaluate.c ../src/generate.c ../src/material.c ../src/move.c
        ../src/move_list.c ../src/move_tree.c ../src/piece.c ../src/search.c ../src/sha1.c ../src/str.c ../src/timer.c
        ../src/global.h

        test_main.cpp
        board_hash_test.cpp
        piece_test.cpp
        coord_test.cpp
        en_passant_test.cpp
        material_test.cpp castle_test.cpp pawn_promotion_test.cpp)

if (HAVE_WINSOCK2_H)
    TARGET_LINK_LIBRARIES(chess_tests ws2_32)
endif (HAVE_WINSOCK2_H)

INCLUDE(ParseAndAddCatchTests)
ParseAndAddCatchTests(chess_tests)