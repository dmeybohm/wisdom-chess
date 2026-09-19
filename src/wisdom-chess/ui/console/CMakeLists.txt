add_executable(wisdom-chess-console main.cpp play.cpp)

# wisdom::viewmodel links wisdom::chess publicly. Naming it again here makes
# the Apple linker warn about a duplicate library.
target_link_libraries(wisdom-chess-console PRIVATE wisdom::viewmodel)
wisdom_chess_enable_warnings(wisdom-chess-console)

if (WISDOM_CHESS_PCH_ENABLED)
    target_precompile_headers(wisdom-chess-console PRIVATE ../../engine/global.hpp)
endif()

install(TARGETS wisdom-chess-console
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
