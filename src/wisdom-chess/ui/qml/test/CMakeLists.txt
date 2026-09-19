# Tests for the QML frontend's C++ classes that need neither a thread nor a
# display. They run under QCoreApplication, so no platform plugin is loaded.

# The QML directory asks for CMake 3.16, which predates test names with spaces.
cmake_policy(SET CMP0110 NEW)

# The application's sources are compiled straight into its executable, which
# a test cannot link, so the ones under test are built again here.
qt_add_library(wisdom-chess-qml-test-support STATIC
    ../main/pieces_model.hpp
    ../main/pieces_model.cpp
    ../main/chess_game.hpp
    ../main/chess_game.cpp
    ../main/game_settings.hpp
    ../main/game_settings.cpp
    ../main/ui_settings.hpp
    ../main/ui_settings.cpp
    ../main/ui_types.hpp
    ../main/ui_types.cpp
)
target_link_libraries(wisdom-chess-qml-test-support
    PUBLIC Qt6::Core Qt6::Qml wisdom::chess wisdom::viewmodel)
target_include_directories(wisdom-chess-qml-test-support PUBLIC ../../../..)
wisdom_chess_enable_warnings(wisdom-chess-qml-test-support)

function(wisdom_chess_add_qml_test test_name source)
    get_filename_component(target_suffix "${source}" NAME_WE)
    string(REPLACE "_" "-" target_suffix "${target_suffix}")
    set(target "wisdom-chess-qml-${target_suffix}")

    qt_add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE wisdom-chess-qml-test-support Qt6::Test)
    wisdom_chess_enable_warnings(${target})

    add_test(NAME "${test_name}" COMMAND ${target})
    set_tests_properties("${test_name}" PROPERTIES LABELS fast)
endfunction()

wisdom_chess_add_qml_test("QML: PiecesModel" pieces_model_test.cpp)
wisdom_chess_add_qml_test("QML: ChessGame" chess_game_test.cpp)
wisdom_chess_add_qml_test("QML: settings and types" settings_test.cpp)
