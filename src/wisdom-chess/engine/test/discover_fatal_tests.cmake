# Writes the CTest file that adds one test for each case of
# wisdom-chess-fatal-tests.
#
# The program lists its own cases, so this runs after each build of it, as
# doctest's discovery does for the other test programs.
#
# Usage: cmake -DFATAL_TEST_EXECUTABLE=<path> -DFATAL_TEST_RUNNER=<path>
#              -DFATAL_TEST_FILE=<path> [-DFATAL_TEST_EMULATOR=<path>]
#              -P discover_fatal_tests.cmake
#
# FATAL_TEST_RUNNER is run_fatal_test.cmake, and FATAL_TEST_FILE the file to
# write.

foreach(required FATAL_TEST_EXECUTABLE FATAL_TEST_RUNNER FATAL_TEST_FILE)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "${required} is not set")
    endif()
endforeach()

execute_process(
    COMMAND ${FATAL_TEST_EMULATOR} "${FATAL_TEST_EXECUTABLE}" --list
    OUTPUT_VARIABLE output
    RESULT_VARIABLE result
)

if(NOT result STREQUAL "0")
    message(FATAL_ERROR "Listing the fatal test cases failed (${result}):\n${output}")
endif()

string(REGEX MATCHALL "[^\r\n]+" cases "${output}")

if(NOT cases)
    message(FATAL_ERROR "${FATAL_TEST_EXECUTABLE} listed no cases.")
endif()

set(tests "")
foreach(case IN LISTS cases)
    string(APPEND tests
        "add_test([==[Fatal: ${case}]==] [==[${CMAKE_COMMAND}]==]\n"
        "    [==[-DFATAL_TEST_EMULATOR=${FATAL_TEST_EMULATOR}]==]\n"
        "    [==[-DFATAL_TEST_EXECUTABLE=${FATAL_TEST_EXECUTABLE}]==]\n"
        "    [==[-DFATAL_TEST_CASE=${case}]==]\n"
        "    -P [==[${FATAL_TEST_RUNNER}]==])\n"
        "set_tests_properties([==[Fatal: ${case}]==] PROPERTIES LABELS fast)\n"
    )
endforeach()

file(WRITE "${FATAL_TEST_FILE}" "${tests}")
