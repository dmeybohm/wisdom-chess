# Runs a command-line frontend with scripted standard input and checks what it
# printed.
#
# Usage: cmake -DCLI_TEST_EXECUTABLE=<path> -DCLI_TEST_INPUT_FILE=<path>
#              -DCLI_TEST_INPUT=<lines separated by |>
#              [-DCLI_TEST_EXPECTED=<regexes separated by @@>]
#              [-DCLI_TEST_FORBIDDEN=<regexes separated by @@>]
#              -P run_cli_test.cmake
#
# The separators are not ";" because add_test() splits arguments there.

foreach(required CLI_TEST_EXECUTABLE CLI_TEST_INPUT_FILE CLI_TEST_INPUT)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "${required} is not set")
    endif()
endforeach()

string(REPLACE "|" "\n" input "${CLI_TEST_INPUT}")
file(WRITE "${CLI_TEST_INPUT_FILE}" "${input}\n")

# Naming the same variable twice merges the two streams.
execute_process(
    COMMAND "${CLI_TEST_EXECUTABLE}"
    INPUT_FILE "${CLI_TEST_INPUT_FILE}"
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
    RESULT_VARIABLE result
    TIMEOUT 120
)

message(STATUS "Input:\n${input}")
message(STATUS "Result: ${result}")
message(STATUS "Output:\n${output}")

if(NOT result STREQUAL "0")
    message(FATAL_ERROR "The process did not exit normally: ${result}")
endif()

# Turn each separator into a list separator, protecting any ";" in a regex.
function(split_regexes text result_variable)
    string(REPLACE ";" "\;" text "${text}")
    string(REPLACE "@@" ";" text "${text}")
    set(${result_variable} "${text}" PARENT_SCOPE)
endfunction()

if(DEFINED CLI_TEST_EXPECTED)
    split_regexes("${CLI_TEST_EXPECTED}" expected_regexes)
    foreach(regex IN LISTS expected_regexes)
        if(NOT output MATCHES "${regex}")
            message(FATAL_ERROR "The output does not match: ${regex}")
        endif()
    endforeach()
endif()

if(DEFINED CLI_TEST_FORBIDDEN)
    split_regexes("${CLI_TEST_FORBIDDEN}" forbidden_regexes)
    foreach(regex IN LISTS forbidden_regexes)
        if(output MATCHES "${regex}")
            message(FATAL_ERROR "The output must not match: ${regex}")
        endif()
    endforeach()
endif()
