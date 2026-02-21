if (WISDOM_CHESS_FAST_TESTS)

    add_executable(fast_tests
        piece_test.cpp
        coord_test.cpp
        en_passant_test.cpp
        material_test.cpp
        castle_test.cpp
        castling_eligibility_test.cpp
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
        generate_test.cpp
        str_test.cpp
        board_test.cpp
        game_test.cpp
        transposition_table_test.cpp
        test_main.cpp)

    target_precompile_headers(fast_tests PRIVATE PRIVATE ../global.hpp)

    target_link_libraries(fast_tests PRIVATE wisdom::chess)
    target_link_libraries(fast_tests PRIVATE doctest::doctest)

    add_test(NAME fast_tests COMMAND fast_tests)

endif()

if (WISDOM_CHESS_SLOW_TESTS)

    add_executable(slow_tests
        search_test.cpp
        perft.cpp
        perft_test.cpp
        move_perft_test.cpp
        hash_collision_slow_test.cpp
        test_main.cpp)

    target_precompile_headers(slow_tests PRIVATE PRIVATE ../global.hpp)

    target_link_libraries(slow_tests PRIVATE wisdom::chess)
    target_link_libraries(slow_tests PRIVATE doctest::doctest)

    add_test(NAME slow_tests COMMAND slow_tests)

    # For perft testing:
    add_executable(perft 
        perft.cpp
        perft_main.cpp)
    target_link_libraries(perft PRIVATE wisdom::chess)

endif()
