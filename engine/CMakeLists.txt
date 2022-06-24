set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.20)

project(WisdomEngine CXX)

find_package(doctest QUIET)
find_package(Microsoft.GSL QUIET)
find_package(Threads REQUIRED)

if (ANDROID)
    # Disable precompile headers because there is a confict on android where it links
    # some code as PIC and some as non-PIC and can't combine them.
    set(PCH_ENABLED OFF)
    message("-- wisdom-chess; ; precompiled headers disabled")
else()
    set(PCH_ENABLED ON)
    message("-- wisdom-chess; ; precompiled headers enabled")
endif()

if (NOT WIN32)
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

if (Microsoft.GSL_DIR)
    message("-- Found Microsoft.GSL")
    set(Microsoft.GSL_Found ON)
else()
    message("-- Using bundled GSL")
    set(Microsoft.GSL_Found OFF)
endif()

add_subdirectory(src)