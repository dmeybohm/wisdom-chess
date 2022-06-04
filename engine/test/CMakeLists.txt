add_library(fast_tests_lib OBJECT
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
        move_parse_test.cpp
        fen_parser_test.cpp
        move_list_test.cpp
        board_code_test.cpp
        history_test.cpp
        tests.hpp
        generate_test.cpp
        uuid_test.cpp
        str_test.cpp
        perft_test.cpp board_test.cpp)

add_library(slow_tests_lib OBJECT
        search_test.cpp
        tests.hpp
        move_perft_test.cpp)

target_precompile_headers(fast_tests_lib PRIVATE PRIVATE ../src/global.hpp <doctest/doctest.h>)
target_precompile_headers(slow_tests_lib REUSE_FROM fast_tests_lib)

target_link_libraries(fast_tests_lib PUBLIC wisdom::chess)
target_link_libraries(slow_tests_lib PUBLIC wisdom::chess)
target_link_libraries(fast_tests_lib PUBLIC doctest::doctest)
target_link_libraries(slow_tests_lib PUBLIC doctest::doctest)

add_executable(fast_tests test_main.cpp)
add_executable(slow_tests test_main.cpp)

target_link_libraries(fast_tests PRIVATE fast_tests_lib)
target_link_libraries(slow_tests PRIVATE slow_tests_lib)
