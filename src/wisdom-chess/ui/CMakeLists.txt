if (WISDOM_CHESS_CONSOLE_UI AND NOT EMSCRIPTEN)
    add_subdirectory(console)
    add_subdirectory(uci)
endif()

if (WISDOM_CHESS_QT_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${WISDOM_CHESS_QT_DIR})
    message("-- wisdom-chess: WISDOM_CHESS_QT_DIR added to CMAKE_PREFIX_PATH")
endif()

# Handle QML UI based on the three-state option
if(WISDOM_CHESS_QML_UI STREQUAL "OFF")
    message("-- wisdom-chess: QML UI disabled by user")
else()
    find_package(Qt6 QUIET COMPONENTS Quick)

    if(Qt6_FOUND)
        message("-- wisdom-chess: Qt6 found. Building QML UI.")
        add_subdirectory(qml)
    elseif(WISDOM_CHESS_QML_UI STREQUAL "ON")
        message(FATAL_ERROR "-- wisdom-chess: QML UI required but Qt6 not found. Install Qt6 or set WISDOM_CHESS_QT_DIR to Qt installation path.")
    else()
        # AUTO mode - Qt6 not found, skip QML UI
        message("-- wisdom-chess: Qt6 not found. QML UI disabled.")
    endif()
endif()

if (EMSCRIPTEN AND WISDOM_CHESS_REACT_UI)
    add_subdirectory(wasm)
    add_subdirectory(react)
endif()
