set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.23)

project(WisdomEngine CXX)

find_package(doctest CONFIG QUIET)
find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(enumflags CONFIG REQUIRED)

if (ANDROID OR NOT WISDOM_CHESS_PCH_ENABLED)
    # Disable precompile headers because there is a confict on android where it links
    # some code as PIC and some as non-PIC and can't combine them.
    set(PCH_ENABLED OFF)
    message("-- wisdom-chess: precompiled headers disabled")
else()
    set(PCH_ENABLED ON)
    message("-- wisdom-chess: precompiled headers enabled")
endif()

if (NOT WIN32 AND NOT EMSCRIPTEN)
    message("-- wisdom-chess: stack protector disabled for performance")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-stack-protector")
endif()

if (doctest_FOUND)
    message("-- wisdom-chess: Including tests with doctest")
    add_subdirectory(test)
else()
    message("-- wisdom-chess: Tests disabled (doctest not found)")
    set(WISDOM_CHESS_SLOW_TESTS Off)
    set(WISDOM_CHESS_FAST_TESTS Off)
endif()

add_subdirectory(src)
