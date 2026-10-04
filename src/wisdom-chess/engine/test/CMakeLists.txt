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
        move_ordering_test.cpp
        board_code_test.cpp
        history_test.cpp
        generate_test.cpp
        evaluate_test.cpp
        game_status_test.cpp
        move_timer_test.cpp
        game_file_test.cpp
        str_test.cpp
        board_test.cpp
        game_test.cpp
        global_test.cpp
        transposition_table_test.cpp
        logger_test.cpp
        test_main.cpp)

    target_precompile_headers(wisdom-chess-fast-tests PRIVATE ../global.hpp)

    target_link_libraries(wisdom-chess-fast-tests PRIVATE wisdom::chess)
    target_link_libraries(wisdom-chess-fast-tests PRIVATE doctest::doctest)
    wisdom_chess_enable_warnings(wisdom-chess-fast-tests)

    if (EMSCRIPTEN)
        # The transposition table tests outgrow the default heap. Growth
        # slows only JavaScript's access to it, which the tests barely use.
        target_link_options(wisdom-chess-fast-tests PRIVATE
            "SHELL:-s ALLOW_MEMORY_GROWTH=1" -Wno-pthreads-mem-growth)
    endif()

    doctest_discover_tests(wisdom-chess-fast-tests PROPERTIES LABELS fast)

    if (WISDOM_CHESS_FILC_COMPAT)
        target_compile_definitions(wisdom-chess-fast-tests PRIVATE DOCTEST_CONFIG_NO_POSIX_SIGNALS)
    endif()

    # Search scenarios, some run on the clock, so they take seconds where the
    # fast tests take one. Built with the fast tests.
    add_executable(wisdom-chess-medium-tests
        search_test.cpp
        test_main.cpp)

    target_precompile_headers(wisdom-chess-medium-tests PRIVATE ../global.hpp)

    target_link_libraries(wisdom-chess-medium-tests PRIVATE wisdom::chess)
    target_link_libraries(wisdom-chess-medium-tests PRIVATE doctest::doctest)
    wisdom_chess_enable_warnings(wisdom-chess-medium-tests)

    if (EMSCRIPTEN)
        target_link_options(wisdom-chess-medium-tests PRIVATE
            "SHELL:-s ALLOW_MEMORY_GROWTH=1" -Wno-pthreads-mem-growth)
    endif()

    doctest_discover_tests(wisdom-chess-medium-tests PROPERTIES LABELS medium)

    if (WISDOM_CHESS_FILC_COMPAT)
        target_compile_definitions(wisdom-chess-medium-tests PRIVATE DOCTEST_CONFIG_NO_POSIX_SIGNALS)
    endif()

    # The main() and case registry of every fatal test program
    # (cmake/FatalTests.cmake).
    add_library(wisdom-chess-fatal-main STATIC fatal_test_main.cpp)
    target_link_libraries(wisdom-chess-fatal-main PUBLIC wisdom::chess)
    wisdom_chess_enable_warnings(wisdom-chess-fatal-main)

    if (WISDOM_CHESS_FILC_COMPAT)
        target_compile_definitions(wisdom-chess-fatal-main PRIVATE WISDOM_CHESS_FILC_COMPAT)
    endif()

    wisdom_chess_add_fatal_tests(wisdom-chess-fatal-tests
        fatal_board_test.cpp
        fatal_contract_test.cpp
        fatal_game_test.cpp
        fatal_move_list_test.cpp
        fatal_numeric_cast_test.cpp
        fatal_parse_test.cpp
        fatal_transposition_table_test.cpp
        fatal_uncaught_test.cpp)

endif()

if (WISDOM_CHESS_SLOW_TESTS)

    add_executable(wisdom-chess-slow-tests
        perft.cpp
        perft_test.cpp
        move_perft_test.cpp
        hash_collision_slow_test.cpp
        test_main.cpp)

    target_precompile_headers(wisdom-chess-slow-tests PRIVATE ../global.hpp)

    target_link_libraries(wisdom-chess-slow-tests PRIVATE wisdom::chess)
    target_link_libraries(wisdom-chess-slow-tests PRIVATE doctest::doctest)
    wisdom_chess_enable_warnings(wisdom-chess-slow-tests)

    if (EMSCRIPTEN)
        # The transposition table tests outgrow the default heap. Growth
        # slows only JavaScript's access to it, which the tests barely use.
        target_link_options(wisdom-chess-slow-tests PRIVATE
            "SHELL:-s ALLOW_MEMORY_GROWTH=1" -Wno-pthreads-mem-growth)
    endif()

    doctest_discover_tests(wisdom-chess-slow-tests PROPERTIES LABELS slow)

    if (WISDOM_CHESS_FILC_COMPAT)
        target_compile_definitions(wisdom-chess-slow-tests PRIVATE DOCTEST_CONFIG_NO_POSIX_SIGNALS)
    endif()

    # For perft testing:
    add_executable(perft
        perft.cpp
        perft_main.cpp)
    target_link_libraries(perft PRIVATE wisdom::chess)
    wisdom_chess_enable_warnings(perft)

endif()
