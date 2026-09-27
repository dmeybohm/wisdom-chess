add_executable(wisdom-chess-web-tests
    ../web_game.cpp
    web_game_test.cpp
    test_main.cpp)

target_link_libraries(wisdom-chess-web-tests PRIVATE wisdom::chess wisdom::viewmodel)
target_link_libraries(wisdom-chess-web-tests PRIVATE doctest::doctest)
wisdom_chess_enable_warnings(wisdom-chess-web-tests)
if(MSVC)
    # doctest 2.4.12 specializes std::tuple, which MSVC 14.51 warns about
    # with C5285 even though the specialization is in a dependency header.
    target_compile_options(wisdom-chess-web-tests PRIVATE /wd5285)
endif()

doctest_discover_tests(wisdom-chess-web-tests PROPERTIES LABELS fast)

if (WISDOM_CHESS_FILC_COMPAT)
    target_compile_definitions(wisdom-chess-web-tests PRIVATE DOCTEST_CONFIG_NO_POSIX_SIGNALS)
endif()
