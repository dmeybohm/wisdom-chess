add_library(wisdom-chess-core STATIC
        board.cpp check.cpp evaluate.cpp game.cpp game.hpp generate.cpp material.cpp move.cpp
        piece.cpp search.cpp str.cpp move_timer.cpp
        global.hpp validate.cpp validate.hpp position.cpp position.hpp
        fen_parser.cpp fen_parser.hpp coord.cpp board_builder.cpp move_list.cpp move_list.hpp
        coord_iterator.hpp analytics_sqlite.hpp history.hpp history.cpp search.hpp
        board_code.cpp board_code.hpp logger.cpp logger.hpp
        transposition_table.cpp transposition_table.hpp variation_glimpse.cpp variation_glimpse.hpp
        output_format.cpp output_format.hpp analytics.cpp analytics.hpp search_result.hpp
        uuid.cpp uuid.hpp play.hpp play.cpp perft.cpp perft.hpp threats.hpp)

if (SQLite3_Found)
    target_sources(wisdom-chess-core PRIVATE analytics_sqlite.cpp)
    target_link_libraries(wisdom-chess-core PUBLIC SQLite::SQLite3)
else()
    target_sources(wisdom-chess-core PRIVATE analytics_nosqlite.cpp)
endif()

if (Microsoft.GSL_Found)
    target_link_libraries(wisdom-chess-core PUBLIC Microsoft.GSL::GSL)
    target_compile_definitions(wisdom-chess-core PUBLIC HAS_MS_GSL)
else()
    target_sources(wisdom-chess-core PRIVATE gsl.hpp)
endif()

if (PCH_ENABLED)
    target_precompile_headers(wisdom-chess-core PRIVATE PRIVATE global.hpp)
else()
    target_sources(wisdom-chess-core PRIVATE global.hpp)
endif()

target_include_directories(wisdom-chess-core PUBLIC .)
add_library(wisdom::chess ALIAS wisdom-chess-core)

add_executable(chess main.cpp)

if (PCH_ENABLED)
    target_precompile_headers(chess REUSE_FROM wisdom-chess-core)
endif()

target_link_libraries(wisdom-chess-core PUBLIC Threads::Threads)

if (EMSCRIPTEN)
    target_compile_options(wisdom-chess-core PUBLIC -pthread)
endif()

target_link_libraries(chess PRIVATE wisdom::chess)

# For perft testing:
if (WISDOM_CHESS_SLOW_TESTS)
    add_executable(perft perft_main.cpp)
    target_link_libraries(perft PRIVATE wisdom::chess)
endif()

