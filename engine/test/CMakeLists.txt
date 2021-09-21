#
# Tests
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake.modules/")

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

target_precompile_headers(chess_tests_lib PRIVATE PRIVATE ../src/global.hpp <doctest/doctest.h>)

# Make test executable
add_executable(chess_tests
        test_main.cpp
        $<TARGET_OBJECTS:chess_lib>
        $<TARGET_OBJECTS:chess_tests_lib>)

target_link_libraries(chess_tests_lib PUBLIC gsl::gsl-lite)
target_link_libraries(chess_tests_lib PRIVATE doctest::doctest)
target_link_libraries(chess_tests_lib PRIVATE SQLite::SQLite3)
target_link_libraries(chess_tests PRIVATE Threads::Threads)
target_link_libraries(chess_tests PRIVATE doctest::doctest)
