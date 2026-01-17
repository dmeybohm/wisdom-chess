add_subdirectory(engine)
add_subdirectory(ui)

if (WISDOM_CHESS_TOOLS AND NOT EMSCRIPTEN)
    add_subdirectory(tools)
endif()
