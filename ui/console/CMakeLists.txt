add_executable(wisdom-chess-console main.cpp play.cpp play.hpp)

target_link_libraries(wisdom-chess-console PRIVATE wisdom::chess)

if (PCH_ENABLED)
    target_precompile_headers(chess REUSE_FROM wisdom-chess-core)
endif()

target_link_libraries(wisdom-chess-console PRIVATE wisdom::chess)

install(TARGETS wisdom-chess-console
    RUNTIME DESTINATION bin)

