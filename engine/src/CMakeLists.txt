add_library(wisdom-chess-core STATIC
        board.cpp board.hpp
        board_builder.cpp board_builder.hpp
        board_code.cpp board_code.hpp
        check.cpp check.hpp
        coord.cpp coord.hpp
        evaluate.cpp evaluate.hpp
        fen_parser.cpp fen_parser.hpp
        game.cpp game.hpp
        game_status.cpp game_status.hpp
        generate.cpp generate.hpp
        history.cpp history.hpp
        logger.cpp logger.hpp
        material.cpp material.hpp
        move.cpp move.hpp
        move_list.cpp move_list.hpp
        move_timer.cpp move_timer.hpp
        output_format.cpp output_format.hpp
        piece.cpp piece.hpp
        position.cpp position.hpp
        search.cpp search.hpp
        str.cpp str.hpp
        global.hpp
        random.hpp
        threats.hpp)

if (PCH_ENABLED)
    target_precompile_headers(wisdom-chess-core PRIVATE PRIVATE global.hpp)
else()
    target_sources(wisdom-chess-core PRIVATE global.hpp)
endif()

target_include_directories(wisdom-chess-core PUBLIC .)
add_library(wisdom::chess ALIAS wisdom-chess-core)

target_link_libraries(wisdom-chess-core PUBLIC Microsoft.GSL::GSL)
target_link_libraries(wisdom-chess-core PUBLIC Threads::Threads)
target_link_libraries(wisdom-chess-core PUBLIC EnumFlags::EnumFlags)

if (EMSCRIPTEN)
    target_compile_options(wisdom-chess-core PUBLIC -pthread)
endif()

if (NOT WIN32 AND NOT EMSCRIPTEN)
    message("-- wisdom-chess: stack protector disabled for performance")
    target_compile_options(wisdom-chess-core PUBLIC -fno-stack-protector)
endif()

