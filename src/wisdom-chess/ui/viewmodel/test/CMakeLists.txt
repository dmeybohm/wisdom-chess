add_executable(wisdom-chess-viewmodel-tests
    game_viewmodel_base_test.cpp
    viewmodel_types_test.cpp
    test_main.cpp)

target_link_libraries(wisdom-chess-viewmodel-tests PRIVATE wisdom::viewmodel)
target_link_libraries(wisdom-chess-viewmodel-tests PRIVATE doctest::doctest)
wisdom_chess_enable_warnings(wisdom-chess-viewmodel-tests)

doctest_discover_tests(wisdom-chess-viewmodel-tests PROPERTIES LABELS fast)

if (WISDOM_CHESS_FILC_COMPAT)
    target_compile_definitions(wisdom-chess-viewmodel-tests PRIVATE DOCTEST_CONFIG_NO_POSIX_SIGNALS)
endif()
