find_package(SQLite3 REQUIRED)
find_package(Microsoft.GSL REQUIRED)
find_package(argh REQUIRED)

add_library(wisdom-chess-core STATIC
        board.cpp check.cpp evaluate.cpp game.cpp game.hpp generate.cpp material.cpp move.cpp
        move_tree.cpp piece.cpp search.cpp str.cpp move_timer.cpp
        global.hpp validate.cpp validate.hpp position.cpp position.hpp
        fen_parser.cpp fen_parser.hpp coord.cpp board_builder.cpp move_list.cpp move_list.hpp
        coord_iterator.hpp
        board_code.cpp board_code.hpp logger.cpp logger.hpp
        transposition_table.cpp transposition_table.hpp variation_glimpse.cpp variation_glimpse.hpp
        output_format.cpp output_format.hpp analytics.cpp analytics.hpp search_result.hpp
        analytics_sqlite.cpp analytics_sqlite.hpp
        uuid.cpp uuid.hpp play.hpp play.cpp perft.cpp perft.hpp threats.cpp threats.hpp)

target_precompile_headers(wisdom-chess-core PRIVATE PRIVATE global.hpp)

target_include_directories(wisdom-chess-core PUBLIC .)
add_library(wisdom::chess ALIAS wisdom-chess-core)

add_executable(chess main.cpp)

target_precompile_headers(chess REUSE_FROM wisdom-chess-core)

target_link_libraries(wisdom-chess-core PUBLIC Microsoft.GSL::GSL)
target_link_libraries(wisdom-chess-core PUBLIC SQLite::SQLite3)
target_link_libraries(chess PRIVATE wisdom::chess)
target_link_libraries(chess PRIVATE argh::argh)

# For perft testing:
add_executable(perft perft_main.cpp)
target_link_libraries(perft PRIVATE wisdom::chess)

