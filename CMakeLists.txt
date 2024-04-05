set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.20)

include(CPM.cmake)

project(WisdomChess CXX)

option(WISDOM_CHESS_ASAN "Enable memory debugging of chess engine")
option(WISDOM_CHESS_FAST_TESTS "Enable building of fast tests" On)
option(WISDOM_CHESS_SLOW_TESTS "Enable building of slow tests")
option(WISDOM_CHESS_CONSOLE_UI "Enable building of console chess game" On)
option(WISDOM_CHESS_QML_UI "Enable building of QML UI" On)
option(WISDOM_CHESS_QT_DIR "Directory for Qt libraries")
option(WISDOM_CHESS_REACT_UI "Enable building of React UI" On)
option(WISDOM_CHESS_PCH_ENABLED "Enable Pre-compiled headers" On)

# Enable -O3 in RelWithDebugInfo
if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message("-- wisdom-chess: Re-enabling -O3 option for G++/Clang")
        string(REPLACE "-O2" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}") 
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3") 
    endif()
endif()

if (WISDOM_CHESS_ASAN)
    message("-- wisdom-chess: Enabling address sanitizer")
    if (MSVC)
        add_compile_options(/fsanitize=address)
        add_link_options(/fsanitize=address /zi)
    else()
        add_compile_options(-fsanitize=address,undefined)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

add_subdirectory(engine)
add_subdirectory(ui)

