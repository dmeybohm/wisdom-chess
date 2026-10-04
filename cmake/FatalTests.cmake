# Fatal errors end the process, so each case runs as its own test, through
# run_fatal_test.cmake. A case passes when the emergency logger reported the
# expected message and the process then terminated abnormally.
#
# wisdom_chess_add_fatal_tests(<target> <source>...)
#
# Builds a program from the sources, whose cases are written with
# FATAL_CASE() (engine/test/fatal_test.hpp), and adds a "Fatal: <case>" test
# for each. The cases register themselves, so they are compiled into the
# program rather than linked from a library, which could drop them. The
# main() they share is the wisdom-chess-fatal-main library.

function(wisdom_chess_add_fatal_tests target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE wisdom-chess-fatal-main)
    wisdom_chess_enable_warnings(${target})

    # Each build of the program asks it for its cases and writes the tests
    # for CTest to include. The cases and the program's path differ between
    # configurations, so a generator with several keeps a file for each, and
    # CTest includes the one that -C names.
    set(tests_base "${CMAKE_CURRENT_BINARY_DIR}/${target}_tests")
    set(include_file "${CMAKE_CURRENT_BINARY_DIR}/${target}_include.cmake")

    get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if (is_multi_config)
        set(tests_file "${tests_base}-$<CONFIG>.cmake")
        set(tests_included "${tests_base}-\${CTEST_CONFIGURATION_TYPE}.cmake")
    else()
        set(tests_file "${tests_base}.cmake")
        set(tests_included "${tests_file}")
    endif()

    add_custom_command(
        TARGET ${target} POST_BUILD
        BYPRODUCTS "${tests_file}"
        COMMAND ${CMAKE_COMMAND}
            "-DFATAL_TEST_EMULATOR=${CMAKE_CROSSCOMPILING_EMULATOR}"
            "-DFATAL_TEST_EXECUTABLE=$<TARGET_FILE:${target}>"
            "-DFATAL_TEST_RUNNER=${CMAKE_CURRENT_FUNCTION_LIST_DIR}/run_fatal_test.cmake"
            "-DFATAL_TEST_FILE=${tests_file}"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/discover_fatal_tests.cmake"
        VERBATIM
    )

    file(WRITE "${include_file}"
        "if(EXISTS \"${tests_included}\")\n"
        "  include(\"${tests_included}\")\n"
        "else()\n"
        "  add_test(${target}_NOT_BUILT ${target}_NOT_BUILT)\n"
        "endif()\n"
    )
    set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES "${include_file}")
endfunction()
