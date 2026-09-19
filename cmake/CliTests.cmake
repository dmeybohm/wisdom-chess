# Tests for the frontends that talk over standard input and output.
#
#   wisdom_chess_add_cli_test(<name> <target>
#       INPUT <line>...
#       [EXPECTED <regex>...]
#       [FORBIDDEN <regex>...])
#
# Feeds the lines to the target's executable and passes when it exits
# normally, its output matches every EXPECTED regex and none of the
# FORBIDDEN ones. See run_cli_test.cmake.

set(WISDOM_CHESS_CLI_TEST_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/run_cli_test.cmake")

function(wisdom_chess_add_cli_test test_name target)
    cmake_parse_arguments(PARSE_ARGV 2 arg "" "" "INPUT;EXPECTED;FORBIDDEN")

    string(MAKE_C_IDENTIFIER "${test_name}" file_name)
    list(JOIN arg_INPUT "|" input)

    set(definitions
        "-DCLI_TEST_EXECUTABLE=$<TARGET_FILE:${target}>"
        "-DCLI_TEST_INPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/${file_name}.input.txt"
        "-DCLI_TEST_INPUT=${input}"
    )
    if(arg_EXPECTED)
        list(JOIN arg_EXPECTED "@@" expected)
        list(APPEND definitions "-DCLI_TEST_EXPECTED=${expected}")
    endif()
    if(arg_FORBIDDEN)
        list(JOIN arg_FORBIDDEN "@@" forbidden)
        list(APPEND definitions "-DCLI_TEST_FORBIDDEN=${forbidden}")
    endif()

    add_test(
        NAME "${test_name}"
        COMMAND ${CMAKE_COMMAND} ${definitions} -P "${WISDOM_CHESS_CLI_TEST_SCRIPT}"
    )
    set_tests_properties("${test_name}" PROPERTIES LABELS fast TIMEOUT 180)
endfunction()
