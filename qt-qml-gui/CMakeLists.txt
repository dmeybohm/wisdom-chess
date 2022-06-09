cmake_minimum_required(VERSION 3.16)

project(WisdomChessQtQml VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 COMPONENTS Quick REQUIRED)

qt_add_executable(appWisdomChessQtQml
    main.cpp
    gamemodel.hpp
    gamemodel.cpp
    piecesmodel.hpp
    piecesmodel.cpp
    chessgame.hpp
    chessgame.cpp
    chessengine.hpp
    chessengine.cpp
    chesscolor.cpp
    chesscolor.hpp
    gamestate.hpp
)

if(ANDROID)
    set(MAIN_QML_FILE mobile_main.qml)
elseif(EMSCRIPTEN)
    set(MAIN_QML_FILE wasm_main.qml)
else()
    set(MAIN_QML_FILE desktop_main.qml)
endif()

qt_add_qml_module(appWisdomChessQtQml
    URI WisdomChessQtQml
    VERSION 1.0
    QML_FILES
        desktop_main.qml
        mobile_main.qml
        wasm_main.qml
        Board.qml
        BoardDimensions.qml
        ChessSquare.qml
        Dialogs.qml
        Piece.qml
        PromoteDropdown.qml
        PromotedPieceModel.qml
        DrawProposalDialog.qml
        NewGameDialog.qml
        SettingsMenu.qml
        StatusBar.qml
        ImageToolButton.qml
        MobileRoot.qml
        DesktopRoot.qml
        Helper.js
    RESOURCES
        images/Chess_bdt45.svg
        images/Chess_blt45.svg
        images/Chess_kdt45.svg
        images/Chess_klt45.svg
        images/Chess_ndt45.svg
        images/Chess_nlt45.svg
        images/Chess_pdt45.svg
        images/Chess_plt45.svg
        images/Chess_qdt45.svg
        images/Chess_qlt45.svg
        images/Chess_rdt45.svg
        images/Chess_rlt45.svg
        images/bx-icon-menu.svg
        images/bx-icon-menu-white.svg
        images/bx-icon-menu-white.png
        images/bxs-down-arrow.svg
)

set_target_properties(appWisdomChessQtQml PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(ANDROID OR EMSCRIPTEN)
    target_compile_definitions(appWisdomChessQtQml PRIVATE USE_DEFAULT_GRAPHICS_BACKEND)
else()
    target_compile_definitions(appWisdomChessQtQml PRIVATE USE_OPENGL_GRAPHICS_BACKEND)
endif()

target_compile_definitions(appWisdomChessQtQml
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(appWisdomChessQtQml
    PRIVATE Qt6::Quick)
target_link_libraries(appWisdomChessQtQml
    PRIVATE wisdom::chess)
target_compile_definitions(appWisdomChessQtQml
    PRIVATE MAIN_QML_FILE="${MAIN_QML_FILE}")
