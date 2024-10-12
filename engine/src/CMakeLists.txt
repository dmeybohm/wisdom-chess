add_library(wisdom-chess-core STATIC
        ../../include/wisdom-chess/engine/board_builder.hpp
        ../../include/wisdom-chess/engine/board_code.hpp
        ../../include/wisdom-chess/engine/board.hpp
        ../../include/wisdom-chess/engine/coord.hpp
        ../../include/wisdom-chess/engine/evaluate.hpp
        ../../include/wisdom-chess/engine/fen_parser.hpp
        ../../include/wisdom-chess/engine/game.hpp
        ../../include/wisdom-chess/engine/game_status.hpp
        ../../include/wisdom-chess/engine/generate.hpp
        ../../include/wisdom-chess/engine/global.hpp
        ../../include/wisdom-chess/engine/history.hpp
        ../../include/wisdom-chess/engine/logger.hpp
        ../../include/wisdom-chess/engine/material.hpp
        ../../include/wisdom-chess/engine/move.hpp
        ../../include/wisdom-chess/engine/move_list.hpp
        ../../include/wisdom-chess/engine/move_timer.hpp
        ../../include/wisdom-chess/engine/output_format.hpp
        ../../include/wisdom-chess/engine/piece.hpp
        ../../include/wisdom-chess/engine/position.hpp
        ../../include/wisdom-chess/engine/random.hpp
        ../../include/wisdom-chess/engine/search.hpp
        ../../include/wisdom-chess/engine/str.hpp
        ../../include/wisdom-chess/engine/threats.hpp
        board.cpp
        board_code.cpp
        coord.cpp
        evaluate.cpp
        fen_parser.cpp
        game.cpp
        game_status.cpp
        generate.cpp
        history.cpp
        logger.cpp 
        material.cpp 
        move.cpp
        move_list.cpp
        move_timer.cpp 
        output_format.cpp
        piece.cpp 
        position.cpp 
        search.cpp
        str.cpp)

if (PCH_ENABLED)
    target_precompile_headers(wisdom-chess-core PRIVATE PRIVATE 
        ../../include/wisdom-chess/engine/global.hpp)
else()
    target_sources(wisdom-chess-core PRIVATE 
        ../../include/wisdom-chess/engine/global.hpp)
endif()

target_include_directories(wisdom-chess-core PRIVATE .)
target_include_directories(wisdom-chess-core PUBLIC ../../include)
add_library(wisdom::chess ALIAS wisdom-chess-core)

target_link_libraries(wisdom-chess-core PUBLIC Microsoft.GSL::GSL)
target_link_libraries(wisdom-chess-core PUBLIC Threads::Threads)
target_link_libraries(wisdom-chess-core PUBLIC type_safe)

if (EMSCRIPTEN)
    target_compile_options(wisdom-chess-core PUBLIC -pthread)
endif()

if (NOT WIN32 AND NOT EMSCRIPTEN)
    message("-- wisdom-chess: stack protector disabled for performance")
    target_compile_options(wisdom-chess-core PUBLIC -fno-stack-protector)
endif()

