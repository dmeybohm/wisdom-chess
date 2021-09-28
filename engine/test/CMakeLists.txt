find_package(doctest REQUIRED)

# Make test executable
add_library(chess_tests_lib OBJECT
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
        fen_parser_test.cpp
        move_list_test.cpp
        board_code_test.cpp
        history_test.cpp
        transposition_table_test.cpp
        assert_test.cpp
        variation_glimpse_test.cpp tests.hpp generate_test.cpp)

# Define main method separate to avoid precompiled doctest header:
add_executable(chess_tests test_main.cpp)

target_precompile_headers(chess_tests_lib PRIVATE PRIVATE ../src/global.hpp <doctest/doctest.h>)

target_link_libraries(chess_tests_lib PUBLIC wisdom::chess)
target_link_libraries(chess_tests_lib PUBLIC doctest::doctest)
target_link_libraries(chess_tests PRIVATE chess_tests_lib)
