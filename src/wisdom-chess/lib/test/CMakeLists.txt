if (WISDOM_CHESS_FAST_TESTS)

    add_executable(wisdom-chess-lib-tests
        cast_test.cpp
        contract_test.cpp
        logger_test.cpp
        optional_test.cpp
        ptr_test.cpp
        random_test.cpp
        str_test.cpp
        test_main.cpp)

    target_link_libraries(wisdom-chess-lib-tests PRIVATE wisdom::lib)
    target_link_libraries(wisdom-chess-lib-tests PRIVATE doctest::doctest)
    wisdom_chess_enable_warnings(wisdom-chess-lib-tests)

    doctest_discover_tests(wisdom-chess-lib-tests PROPERTIES LABELS fast)

    if (WISDOM_CHESS_FILC_COMPAT)
        target_compile_definitions(wisdom-chess-lib-tests PRIVATE DOCTEST_CONFIG_NO_POSIX_SIGNALS)
    endif()

    # The main() and case registry of every fatal test program
    # (cmake/FatalTests.cmake).
    add_library(wisdom-chess-fatal-main STATIC fatal_test_main.cpp)
    target_link_libraries(wisdom-chess-fatal-main PUBLIC wisdom::lib)
    wisdom_chess_enable_warnings(wisdom-chess-fatal-main)

    if (WISDOM_CHESS_FILC_COMPAT)
        target_compile_definitions(wisdom-chess-fatal-main PRIVATE WISDOM_CHESS_FILC_COMPAT)
    endif()

    wisdom_chess_add_fatal_tests(wisdom-chess-lib-fatal-tests
        fatal_cast_test.cpp
        fatal_contract_test.cpp)

endif()
