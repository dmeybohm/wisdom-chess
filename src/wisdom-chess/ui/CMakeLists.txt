# Shared viewmodel base library (used by QML and WASM)
add_subdirectory(viewmodel)

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

    if(NOT Qt6_FOUND AND UNIX AND NOT APPLE AND NOT EMSCRIPTEN)
        # Prebuilt Qt on Linux hard-requires OpenGL development files at
        # configure time (Qt6Gui -> WrapOpenGL), and a missing libglvnd is
        # the usual reason Qt6 "isn't found" on a machine that has Qt.
        find_package(OpenGL QUIET)
        if(NOT OpenGL_FOUND)
            message(WARNING
                "-- wisdom-chess: OpenGL development files were not found; "
                "Qt6 cannot be configured without them. Install libglvnd-dev "
                "(Debian/Ubuntu) or libglvnd-devel (Fedora) and re-run CMake.")
        endif()
    endif()

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
