cmake_minimum_required(VERSION 3.16)

project(WisdomChessQt6 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 COMPONENTS Quick REQUIRED)

qt_add_executable(appWisdomChessQt6
    main.cpp
    gamemodel.h
    gamemodel.cpp
    gamethread.h
    gamethread.cpp
    resources.qrc
)

qt_add_qml_module(appWisdomChessQt6
    URI WisdomChessQt6
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

set_target_properties(appWisdomChessQt6 PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_compile_definitions(appWisdomChessQt6
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(appWisdomChessQt6
    PRIVATE Qt6::Quick)
target_link_libraries(appWisdomChessQt6
    PRIVATE wisdom::chess)
