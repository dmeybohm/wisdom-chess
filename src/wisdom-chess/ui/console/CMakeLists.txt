add_executable(wisdom-chess-console main.cpp play.cpp)

target_link_libraries(wisdom-chess-console PRIVATE wisdom::chess wisdom::viewmodel)
wisdom_chess_enable_warnings(wisdom-chess-console)

if (WISDOM_CHESS_PCH_ENABLED)
    target_precompile_headers(wisdom-chess-console REUSE_FROM wisdom-chess-core)
endif()

install(TARGETS wisdom-chess-console
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
