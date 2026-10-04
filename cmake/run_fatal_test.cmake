# Runs one case of a fatal test program (FatalTests.cmake) and judges how it
# ended.
#
# CTest fails any test whose process dies from a signal, whatever it printed,
# and not every toolchain lets the process catch its own abort. Running the
# case from a script makes the verdict independent of how the process dies.
#
# Usage: cmake -DFATAL_TEST_EXECUTABLE=<path> -DFATAL_TEST_CASE=<name>
#              [-DFATAL_TEST_EMULATOR=<path>] -P run_fatal_test.cmake
#
# FATAL_TEST_EMULATOR runs the executable when cross-compiling, such as node
# for Emscripten. The case prints the message it expects, as a regular
# expression, before it triggers the error.

foreach(required FATAL_TEST_EXECUTABLE FATAL_TEST_CASE)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "${required} is not set")
    endif()
endforeach()

# Naming the same variable twice merges the two streams.
execute_process(
    COMMAND ${FATAL_TEST_EMULATOR} "${FATAL_TEST_EXECUTABLE}" "${FATAL_TEST_CASE}"
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
    RESULT_VARIABLE result
)

message(STATUS "Result: ${result}")
message(STATUS "Output:\n${output}")

if(NOT output MATCHES "\\[expecting\\] ([^\r\n]*)")
    message(FATAL_ERROR "The process did not run the case.")
endif()
set(expected "${CMAKE_MATCH_1}")

if(output MATCHES "\\[survived\\]")
    message(FATAL_ERROR "The process carried on past the fatal error.")
endif()

if(result STREQUAL "0")
    message(FATAL_ERROR "The process exited normally instead of terminating.")
endif()

if(NOT output MATCHES "\\[emergency\\] ${expected}")
    message(FATAL_ERROR "The emergency logger did not report: ${expected}")
endif()
