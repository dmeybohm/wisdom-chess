#
# Tests
#

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake.modules/")

add_executable(chess_tests
        ../src/board.cpp ../src/board_hash.cpp ../src/check.cpp ../src/debug.cpp ../src/evaluate.cpp ../src/generate.cpp
        ../src/material.cpp ../src/move.cpp ../src/move_tree.cpp ../src/piece.cpp ../src/search.cpp
        ../src/sha1.cpp ../src/str.cpp ../src/timer.cpp ../src/global.h ../src/board_check.cpp ../src/board_check.h
        ../src/validate.cpp ../src/position.cpp ../src/board_builder.cpp ../src/game.cpp ../src/game.h
        ../src/board_builder.hpp

        test_main.cpp
        board_hash_test.cpp
        piece_test.cpp
        coord_test.cpp
        en_passant_test.cpp
        material_test.cpp
        castle_test.cpp
        pawn_promotion_test.cpp
        board_iterator_test.cpp
        board_builder_test.cpp
        check_test.cpp position_test.cpp move_test.cpp search_test.cpp parse_simple_move.cpp parse_simple_move.hpp
        parse_simple_move_test.cpp ../src/realloc_vector.cpp ../src/realloc_vector.hpp ../src/coord.cpp
        ../src/fen.cpp ../src/fen.hpp fen_test.cpp)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -O3 -fsanitize=address -march=native")

# Speed up linking on Linux
execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE ld_version)
if ("${ld_version}" MATCHES "GNU gold")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
endif()

if (HAVE_WINSOCK2_H)
    TARGET_LINK_LIBRARIES(chess_tests ws2_32)
endif (HAVE_WINSOCK2_H)

INCLUDE(ParseAndAddCatchTests)
ParseAndAddCatchTests(chess_tests)