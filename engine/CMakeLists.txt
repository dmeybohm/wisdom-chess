set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.20)

project(WisdomEngine CXX)

find_package(Threads REQUIRED)

CPMAddPackage("gh:foonathan/type_safe@0.2.3")
CPMAddPackage("gh:microsoft/GSL@4.0.0")
CPMAddPackage(
  NAME doctest
  GITHUB_REPOSITORY doctest/doctest
  GIT_TAG v2.4.11
)

find_package(doctest)

if (ANDROID OR NOT WISDOM_CHESS_PCH_ENABLED)
    # Disable precompile headers because there is a confict on android where it links
    # some code as PIC and some as non-PIC and can't combine them.
    set(PCH_ENABLED OFF)
    message("-- wisdom-chess: precompiled headers disabled")
else()
    set(PCH_ENABLED ON)
    message("-- wisdom-chess: precompiled headers enabled")
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
