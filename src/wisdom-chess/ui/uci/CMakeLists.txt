target_sources(wisdom-chess-core PRIVATE
    uci_interface.cpp
)

if (WISDOM_CHESS_CONSOLE_UI)
    add_executable(wisdom-chess-uci
        main.cpp
    )

    target_link_libraries(wisdom-chess-uci
        wisdom-chess-core
    )

    if (WISDOM_CHESS_PCH_ENABLED)
        target_precompile_headers(wisdom-chess-uci REUSE_FROM wisdom-chess-core)
    endif()
endif()