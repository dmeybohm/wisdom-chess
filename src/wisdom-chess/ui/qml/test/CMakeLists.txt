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
    ../main/game_model.hpp
    ../main/game_model.cpp
    ../main/chess_engine.hpp
    ../main/chess_engine.cpp
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

# The UI tests load the real QML. The files live in the application
# executable's resources, so they are embedded again in each test executable
# under the same resource paths, from the lists the application's QML module
# recorded.
get_target_property(app_qml_files WisdomChessQml QT_QML_MODULE_QML_FILES)
get_target_property(app_resources WisdomChessQml QT_QML_MODULE_RESOURCES)
get_target_property(app_module_dir WisdomChessQml QT_QML_MODULE_OUTPUT_DIRECTORY)

set(test_resource_files "")
foreach(file IN LISTS app_qml_files app_resources)
    get_filename_component(absolute_file "${file}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
    file(RELATIVE_PATH alias "${CMAKE_CURRENT_SOURCE_DIR}/.." "${absolute_file}")
    set_source_files_properties("${absolute_file}" PROPERTIES QT_RESOURCE_ALIAS "${alias}")
    list(APPEND test_resource_files "${absolute_file}")
endforeach()

# The generated module descriptions, which let one QML file name another.
foreach(qmldir qmldir main/qmldir popups/qmldir)
    set(qmldir_file "${app_module_dir}/${qmldir}")
    set_source_files_properties("${qmldir_file}" PROPERTIES
        QT_RESOURCE_ALIAS "${qmldir}" GENERATED TRUE)
    list(APPEND test_resource_files "${qmldir_file}")
endforeach()

# RESOURCE_FILES, if given, replaces the list of files to embed.
function(wisdom_chess_add_qml_ui_test test_name source)
    cmake_parse_arguments(PARSE_ARGV 2 arg "" "" "RESOURCE_FILES")
    if(NOT arg_RESOURCE_FILES)
        set(arg_RESOURCE_FILES ${test_resource_files})
    endif()

    wisdom_chess_add_qml_test("${test_name}" ${source})

    get_filename_component(target_suffix "${source}" NAME_WE)
    string(REPLACE "_" "-" target_suffix "${target_suffix}")
    set(target "wisdom-chess-qml-${target_suffix}")

    target_link_libraries(${target} PRIVATE Qt6::Quick Qt6::QuickTest Qt6::Svg)
    qt_add_resources(${target} "${target_suffix}-qml"
        PREFIX "/qt/qml/WisdomChess"
        FILES ${arg_RESOURCE_FILES})
    add_dependencies(${target} WisdomChessQml)

    set_tests_properties("${test_name}" PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")
endfunction()

wisdom_chess_add_qml_ui_test("QML: application" application_test.cpp)
wisdom_chess_add_qml_ui_test("QML: dialogs" dialogs_test.cpp)

# The mobile QML belongs to the Android build's module only. Its two files are
# added here, with a module description that also names MobileRoot.
set(mobile_qmldir "${CMAKE_CURRENT_BINARY_DIR}/mobile/qmldir")
add_custom_command(
    OUTPUT "${mobile_qmldir}"
    COMMAND ${CMAKE_COMMAND}
        "-DINPUT_FILE=${app_module_dir}/qmldir"
        "-DOUTPUT_FILE=${mobile_qmldir}"
        "-DEXTRA_LINES=MobileRoot 1.0 main/MobileRoot.qml"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/append_lines.cmake"
    DEPENDS "${app_module_dir}/qmldir" "${CMAKE_CURRENT_SOURCE_DIR}/append_lines.cmake"
    VERBATIM
)
set_source_files_properties("${mobile_qmldir}" PROPERTIES QT_RESOURCE_ALIAS "qmldir")

set(mobile_resource_files ${test_resource_files})
list(REMOVE_ITEM mobile_resource_files "${app_module_dir}/qmldir")
list(APPEND mobile_resource_files "${mobile_qmldir}")
foreach(file main/mobile_main.qml main/MobileRoot.qml)
    set(absolute_file "${CMAKE_CURRENT_SOURCE_DIR}/../${file}")
    set_source_files_properties("${absolute_file}" PROPERTIES QT_RESOURCE_ALIAS "${file}")
    list(APPEND mobile_resource_files "${absolute_file}")
endforeach()

wisdom_chess_add_qml_ui_test("QML: mobile" mobile_test.cpp
    RESOURCE_FILES ${mobile_resource_files})
