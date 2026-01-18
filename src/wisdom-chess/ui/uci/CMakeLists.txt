if (WISDOM_CHESS_CONSOLE_UI)
    add_executable(wisdom-chess-uci
        main.cpp
        uci_interface.cpp
    )

    target_link_libraries(wisdom-chess-uci
        wisdom-chess-core
    )

    # Enable experimental library features for jthread support on AppleClang
    if (APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
        target_compile_options(wisdom-chess-uci PRIVATE -fexperimental-library)
        target_link_options(wisdom-chess-uci PRIVATE -fexperimental-library)
    endif()

    if (WISDOM_CHESS_PCH_ENABLED)
        target_precompile_headers(wisdom-chess-uci REUSE_FROM wisdom-chess-core)
    endif()
endif()
