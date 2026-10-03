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

    # Fatal errors end the process, so each case runs as its own test, through
    # run_fatal_test.cmake. A case passes when the emergency logger reported the
    # expected message and the process then terminated abnormally.
    add_executable(wisdom-chess-fatal-tests fatal_test_main.cpp)
    target_link_libraries(wisdom-chess-fatal-tests PRIVATE wisdom::chess)
    wisdom_chess_enable_warnings(wisdom-chess-fatal-tests)

    if (WISDOM_CHESS_FILC_COMPAT)
        target_compile_definitions(wisdom-chess-fatal-tests PRIVATE WISDOM_CHESS_FILC_COMPAT)
    endif()

    # The cases are listed in fatal_test_main.cpp. Each build of the program
    # asks it for them and writes the tests for CTest to include. The cases
    # and the program's path differ between configurations, so a generator
    # with several keeps a file for each, and CTest includes the one that -C
    # names.
    set(fatal_tests_base "${CMAKE_CURRENT_BINARY_DIR}/wisdom-chess-fatal-tests_tests")
    set(fatal_include_file "${CMAKE_CURRENT_BINARY_DIR}/wisdom-chess-fatal-tests_include.cmake")

    get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if (is_multi_config)
        set(fatal_tests_file "${fatal_tests_base}-$<CONFIG>.cmake")
        set(fatal_tests_included "${fatal_tests_base}-\${CTEST_CONFIGURATION_TYPE}.cmake")
    else()
        set(fatal_tests_file "${fatal_tests_base}.cmake")
        set(fatal_tests_included "${fatal_tests_file}")
    endif()

    add_custom_command(
        TARGET wisdom-chess-fatal-tests POST_BUILD
        BYPRODUCTS "${fatal_tests_file}"
        COMMAND ${CMAKE_COMMAND}
            "-DFATAL_TEST_EMULATOR=${CMAKE_CROSSCOMPILING_EMULATOR}"
            "-DFATAL_TEST_EXECUTABLE=$<TARGET_FILE:wisdom-chess-fatal-tests>"
            "-DFATAL_TEST_RUNNER=${CMAKE_CURRENT_SOURCE_DIR}/run_fatal_test.cmake"
            "-DFATAL_TEST_FILE=${fatal_tests_file}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/discover_fatal_tests.cmake"
        VERBATIM
    )

    file(WRITE "${fatal_include_file}"
        "if(EXISTS \"${fatal_tests_included}\")\n"
        "  include(\"${fatal_tests_included}\")\n"
        "else()\n"
        "  add_test(wisdom-chess-fatal-tests_NOT_BUILT wisdom-chess-fatal-tests_NOT_BUILT)\n"
        "endif()\n"
    )
    set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES "${fatal_include_file}")

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
