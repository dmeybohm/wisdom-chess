if (WISDOM_CHESS_FAST_TESTS)

    add_executable(wisdom-chess-fast-tests
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

    target_precompile_headers(wisdom-chess-fast-tests PRIVATE PRIVATE ../global.hpp)

    target_link_libraries(wisdom-chess-fast-tests PRIVATE wisdom::chess)
    target_link_libraries(wisdom-chess-fast-tests PRIVATE doctest::doctest)

    doctest_discover_tests(wisdom-chess-fast-tests PROPERTIES LABELS fast)

    if (WISDOM_CHESS_FILC_COMPAT)
        target_compile_definitions(wisdom-chess-fast-tests PRIVATE DOCTEST_CONFIG_NO_POSIX_SIGNALS)
    endif()

endif()

if (WISDOM_CHESS_SLOW_TESTS)

    add_executable(wisdom-chess-slow-tests
        search_test.cpp
        perft.cpp
        perft_test.cpp
        move_perft_test.cpp
        hash_collision_slow_test.cpp
        test_main.cpp)

    target_precompile_headers(wisdom-chess-slow-tests PRIVATE PRIVATE ../global.hpp)

    target_link_libraries(wisdom-chess-slow-tests PRIVATE wisdom::chess)
    target_link_libraries(wisdom-chess-slow-tests PRIVATE doctest::doctest)

    doctest_discover_tests(wisdom-chess-slow-tests PROPERTIES LABELS slow)

    if (WISDOM_CHESS_FILC_COMPAT)
        target_compile_definitions(wisdom-chess-slow-tests PRIVATE DOCTEST_CONFIG_NO_POSIX_SIGNALS)
    endif()

    # For perft testing:
    add_executable(perft 
        perft.cpp
        perft_main.cpp)
    target_link_libraries(perft PRIVATE wisdom::chess)

endif()
