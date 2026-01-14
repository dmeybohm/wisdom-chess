set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.20)

include(cmake/CPM.cmake)

project(WisdomChess CXX)

enable_testing()

option(WISDOM_CHESS_ASAN "Enable memory debugging of chess engine")
option(WISDOM_CHESS_FAST_TESTS "Enable building of fast tests" On)
option(WISDOM_CHESS_SLOW_TESTS "Enable building of slow tests" On)
option(WISDOM_CHESS_CONSOLE_UI "Enable building of console chess game" On)
option(WISDOM_CHESS_REACT_UI "Enable building of React UI" On)
option(WISDOM_CHESS_REACT_BUILD_INTEGRATED "Build React frontend as part of CMake build" ${WISDOM_CHESS_REACT_BUILD_INTEGRATED_DEFAULT})
option(WISDOM_CHESS_PCH_ENABLED "Enable Pre-compiled headers" On)
option(WISDOM_CHESS_TOOLS "Build research/analysis tools" Off)
option(WISDOM_CHESS_BENCHMARKS "Enable building of benchmarks" Off)
option(WISDOM_CHESS_FILC_COMPAT "Enable FIL-c runtime compatibility (disables POSIX signals in doctest)" OFF)

# QML UI option: AUTO (build if Qt6 found), ON (require Qt6), OFF (disable)
set(WISDOM_CHESS_QML_UI "AUTO" CACHE STRING "Build QML UI: AUTO (if Qt6 found), ON (require Qt6), OFF (disable)")
set_property(CACHE WISDOM_CHESS_QML_UI PROPERTY STRINGS AUTO ON OFF)
set(WISDOM_CHESS_QT_DIR "" CACHE PATH "Directory for Qt libraries")

# Set default for integrated React build based on whether it's a WASM build
if(EMSCRIPTEN AND NOT DEFINED WISDOM_CHESS_REACT_BUILD_INTEGRATED)
    set(WISDOM_CHESS_REACT_BUILD_INTEGRATED_DEFAULT ON)
else()
    set(WISDOM_CHESS_REACT_BUILD_INTEGRATED_DEFAULT OFF)
endif()

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

if (WISDOM_CHESS_FILC_COMPAT)
    message("-- wisdom-chess: Enabling FIL-c runtime compatibility")
endif()

add_subdirectory(src)

# C++ style linter (only for native builds, not WASM)
if(NOT EMSCRIPTEN)
    option(WISDOM_CHESS_BUILD_LINTER "Build the C++ style linter" ON)
    if(WISDOM_CHESS_BUILD_LINTER)
        add_subdirectory(scripts/linter)

        add_custom_target(lint
            COMMAND wisdom-linter ${CMAKE_SOURCE_DIR}/src
            DEPENDS wisdom-linter
            COMMENT "Running C++ style linter"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    endif()
endif()
