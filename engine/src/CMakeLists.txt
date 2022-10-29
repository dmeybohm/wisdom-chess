add_library(wisdom-chess-core STATIC
        board.cpp check.cpp evaluate.cpp game.cpp game.hpp generate.cpp material.cpp move.cpp
        piece.cpp search.cpp str.cpp move_timer.cpp
        global.hpp position.cpp position.hpp
        fen_parser.cpp fen_parser.hpp coord.cpp board_builder.cpp move_list.cpp move_list.hpp
        history.hpp history.cpp search.hpp
        board_code.cpp board_code.hpp logger.cpp logger.hpp
        output_format.cpp output_format.hpp
        threats.hpp)

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

target_link_libraries(wisdom-chess-core PUBLIC Threads::Threads)
target_link_libraries(wisdom-chess-core PUBLIC enum-flags::enum-flags)

if (EMSCRIPTEN)
    target_compile_options(wisdom-chess-core PUBLIC -pthread)
endif()

