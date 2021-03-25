#
# Tests
#

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake.modules/")

# Make test executable
add_executable(chess_tests
        test_main.cpp
        piece_test.cpp
        coord_test.cpp
        en_passant_test.cpp
        material_test.cpp
        castle_test.cpp
        pawn_promotion_test.cpp
        board_builder_test.cpp
        check_test.cpp
        position_test.cpp
        move_test.cpp
        search_test.cpp
        parse_simple_move_test.cpp
        fen_test.cpp
        move_list_test.cpp
        board_code_test.cpp
        history_test.cpp
        $<TARGET_OBJECTS:chess_lib>
        transposition_table_test.cpp)