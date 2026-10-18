# General-purpose code the engine and the frontends are built on: the
# standard-type aliases, the contract checks, the named conversions, the
# compact optional, the pointer types, the logger and the string helpers.
# Nothing here knows about chess.

add_library(wisdom-chess-lib STATIC
        cast.hpp
        error.hpp
        expected.hpp
        logger.hpp
        optional.hpp
        ptr.hpp
        random.hpp
        str.hpp
        types.hpp
        error.cpp
        logger.cpp
        str.cpp)

target_include_directories(wisdom-chess-lib PUBLIC ../..)
target_include_directories(wisdom-chess-lib SYSTEM PUBLIC ${expected_SOURCE_DIR}/include)
add_library(wisdom::lib ALIAS wisdom-chess-lib)

target_link_libraries(wisdom-chess-lib PUBLIC Threads::Threads)

wisdom_chess_enable_warnings(wisdom-chess-lib)

if (EMSCRIPTEN)
    target_compile_options(wisdom-chess-lib PUBLIC -pthread)
endif()

if (NOT WIN32 AND NOT EMSCRIPTEN)
    target_compile_options(wisdom-chess-lib PRIVATE -fno-stack-protector)
endif()

if (doctest_FOUND)
    add_subdirectory(test)
endif()
