add_executable(wisdom-chess-console main.cpp play.cpp)

target_link_libraries(wisdom-chess-console PRIVATE wisdom::chess wisdom::viewmodel)

if (PCH_ENABLED)
    target_precompile_headers(chess REUSE_FROM wisdom-chess-core)
endif()

install(TARGETS wisdom-chess-console
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
