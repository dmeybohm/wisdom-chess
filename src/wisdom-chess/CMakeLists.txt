find_package(Threads REQUIRED)

CPMAddPackage(
  NAME doctest
  GITHUB_REPOSITORY doctest/doctest
  GIT_TAG v2.4.12
  OPTIONS
     "DOCTEST_NO_INSTALL On"
)

find_package(doctest)

# Header only. Its own CMake project would add tests and install rules, so
# only its include directory is used.
CPMAddPackage(
  NAME expected
  GITHUB_REPOSITORY TartanLlama/expected
  GIT_TAG v1.3.1
  DOWNLOAD_ONLY YES
)

if (doctest_FOUND)
    include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
    message("-- wisdom-chess: Including tests with doctest")
else()
    message("-- wisdom-chess: Tests disabled (doctest not found)")
    set(WISDOM_CHESS_SLOW_TESTS Off)
    set(WISDOM_CHESS_FAST_TESTS Off)
endif()

if (EMSCRIPTEN AND TARGET doctest)
    # Under node, stdout always looks like a terminal, and the color codes
    # in the test list break doctest_discover_tests().
    target_compile_definitions(doctest INTERFACE DOCTEST_CONFIG_COLORS_NONE)

    # The engine is compiled with -pthread, and a test executable linked
    # without it has no thread-local storage.
    target_link_options(doctest INTERFACE -pthread)
endif()

add_subdirectory(lib)
add_subdirectory(engine)
add_subdirectory(ui)

if (WISDOM_CHESS_TOOLS AND NOT EMSCRIPTEN)
    add_subdirectory(tools)
endif()
