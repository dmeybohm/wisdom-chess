#
# Tests
#

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake.modules/")

add_executable(chess_tests
        ../board.c ../board_hash.c ../check.c ../debug.c ../evaluate.c ../generate.c ../material.c ../move.c
        ../move_list.c ../move_tree.c ../piece.c ../search.c ../sha1.c ../str.c ../timer.c
        ../global.h

        test_main.cpp
        board_hash_test.cpp
        piece_test.cpp
        coord_test.cpp
        material_test.cpp)

if (HAVE_WINSOCK2_H)
    TARGET_LINK_LIBRARIES(chess_tests ws2_32)
endif (HAVE_WINSOCK2_H)

INCLUDE(ParseAndAddCatchTests)
ParseAndAddCatchTests(chess_tests)