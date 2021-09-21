# add the executable
add_library(chess_lib OBJECT
        board.cpp check.cpp evaluate.cpp game.cpp generate.cpp material.cpp move.cpp
        move_tree.cpp piece.cpp search.cpp str.cpp move_timer.cpp
        global.hpp validate.cpp validate.hpp position.cpp position.hpp
        fen_parser.cpp fen_parser.hpp coord.cpp board_builder.cpp move_list.cpp move_list.hpp
        multithread_search.cpp multithread_search.hpp coord_iterator.cpp coord_iterator.hpp
        board_code.cpp board_code.hpp logger.cpp logger.hpp board_history.cpp board_history.hpp
        history.cpp transposition_table.cpp transposition_table.hpp variation_glimpse.cpp variation_glimpse.hpp
        output_format.cpp output_format.hpp analytics.cpp analytics.hpp search_result.cpp search_result.hpp
        uuid.cpp uuid.hpp)
add_executable(chess
        play.cpp
        analytics_sqlite.cpp analytics_sqlite.hpp
        $<TARGET_OBJECTS:chess_lib>)

target_precompile_headers(chess_lib PRIVATE PRIVATE global.hpp)
target_precompile_headers(chess REUSE_FROM chess_lib)

target_link_libraries(chess_lib PUBLIC gsl::gsl-lite)
target_link_libraries(chess_lib PRIVATE SQLite::SQLite3)
target_link_libraries(chess PRIVATE Threads::Threads)
target_link_libraries(chess PRIVATE SQLite::SQLite3)
