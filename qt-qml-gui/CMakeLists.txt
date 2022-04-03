cmake_minimum_required(VERSION 3.16)

project(WisdomChessQtQml VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 COMPONENTS Quick REQUIRED)

qt_add_executable(appWisdomChessQtQml
    main.cpp
    gamemodel.h
    gamemodel.cpp
    chessengine.h
    chessengine.cpp
    chessenginenotifier.cpp
    chessenginenotifier.h
    resources.qrc
)

qt_add_qml_module(appWisdomChessQtQml
    URI WisdomChessQtQml
    VERSION 1.0
    QML_FILES
        main.qml Board.qml ChessSquare.qml PiecesModel.qml Piece.qml
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
)

set_target_properties(appWisdomChessQtQml PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_compile_definitions(appWisdomChessQtQml
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(appWisdomChessQtQml
    PRIVATE Qt6::Quick)
target_link_libraries(appWisdomChessQtQml
    PRIVATE wisdom::chess)
