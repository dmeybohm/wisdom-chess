# Writes OUTPUT_FILE as INPUT_FILE followed by EXTRA_LINES, which are
# separated by "|".
#
# Usage: cmake -DINPUT_FILE=<path> -DOUTPUT_FILE=<path> -DEXTRA_LINES=<a|b>
#              -P append_lines.cmake

foreach(required INPUT_FILE OUTPUT_FILE EXTRA_LINES)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "${required} is not set")
    endif()
endforeach()

file(READ "${INPUT_FILE}" contents)
string(REPLACE "|" "\n" extra "${EXTRA_LINES}")
file(WRITE "${OUTPUT_FILE}" "${contents}${extra}\n")
