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
        evaluate_test.cpp
        game_status_test.cpp
        move_timer_test.cpp
        output_format_test.cpp
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

    doctest_discover_tests(wisdom-chess-fast-tests PROPERTIES LABELS fast)

    if (WISDOM_CHESS_FILC_COMPAT)
        target_compile_definitions(wisdom-chess-fast-tests PRIVATE DOCTEST_CONFIG_NO_POSIX_SIGNALS)
    endif()

    # Fatal errors end the process, so each case runs as its own test, through
    # run_fatal_test.cmake. A case passes when the emergency logger reported the
    # expected message and the process then terminated abnormally.
    add_executable(wisdom-chess-fatal-tests fatal_test_main.cpp)
    target_link_libraries(wisdom-chess-fatal-tests PRIVATE wisdom::chess)
    wisdom_chess_enable_warnings(wisdom-chess-fatal-tests)

    if (WISDOM_CHESS_FILC_COMPAT)
        target_compile_definitions(wisdom-chess-fatal-tests PRIVATE WISDOM_CHESS_FILC_COMPAT)
    endif()

    function(wisdom_chess_add_fatal_test test_case expected_output)
        add_test(
            NAME "Fatal: ${test_case}"
            COMMAND ${CMAKE_COMMAND}
                "-DFATAL_TEST_EXECUTABLE=$<TARGET_FILE:wisdom-chess-fatal-tests>"
                "-DFATAL_TEST_CASE=${test_case}"
                "-DFATAL_TEST_EXPECTED=${expected_output}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/run_fatal_test.cmake"
        )
        set_tests_properties("Fatal: ${test_case}" PROPERTIES LABELS fast)
    endfunction()

    wisdom_chess_add_fatal_test(append-overflow "Precondition failed at .*move_list\\.hpp")
    wisdom_chess_add_fatal_test(remove-from-empty "Precondition failed at .*move_list\\.hpp")
    wisdom_chess_add_fatal_test(bad-castling-flags "Precondition failed at .*castling\\.hpp")
    wisdom_chess_add_fatal_test(bad-en-passant-row "Precondition failed at .*board_code\\.hpp")
    wisdom_chess_add_fatal_test(uncaught-error "Uncaught error: boom")
    wisdom_chess_add_fatal_test(expects-through-noexcept "Uncaught error: Precondition failed at")

endif()

if (WISDOM_CHESS_SLOW_TESTS)

    add_executable(wisdom-chess-slow-tests
        search_test.cpp
        perft.cpp
        perft_test.cpp
        move_perft_test.cpp
        hash_collision_slow_test.cpp
        test_main.cpp)

    target_precompile_headers(wisdom-chess-slow-tests PRIVATE ../global.hpp)

    target_link_libraries(wisdom-chess-slow-tests PRIVATE wisdom::chess)
    target_link_libraries(wisdom-chess-slow-tests PRIVATE doctest::doctest)
    wisdom_chess_enable_warnings(wisdom-chess-slow-tests)

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
