# CopyHashedFiles.cmake - Copy hashed WASM files to destination directory
#
# Usage:
#   cmake -DSOURCE_DIR=<dir> -DDEST_DIR=<dir> -P CopyHashedFiles.cmake
#
# This script copies all hashed WASM files (*.*.wasm, *.*.js patterns excluding
# the loader which has a fixed name) from SOURCE_DIR to DEST_DIR.

if(NOT CMAKE_SCRIPT_MODE_FILE)
    message(FATAL_ERROR "This script must be run with cmake -P")
endif()

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR must be defined")
endif()
if(NOT DEFINED DEST_DIR)
    message(FATAL_ERROR "DEST_DIR must be defined")
endif()

file(GLOB HASHED_WASM_FILES "${SOURCE_DIR}/*.*.wasm")
file(GLOB HASHED_JS_FILES "${SOURCE_DIR}/*.*.js")

file(MAKE_DIRECTORY "${DEST_DIR}")

foreach(FILE ${HASHED_WASM_FILES})
    get_filename_component(FILENAME ${FILE} NAME)
    file(COPY_FILE ${FILE} "${DEST_DIR}/${FILENAME}")
    message(STATUS "Copied ${FILENAME} to public/")
endforeach()

foreach(FILE ${HASHED_JS_FILES})
    get_filename_component(FILENAME ${FILE} NAME)
    file(COPY_FILE ${FILE} "${DEST_DIR}/${FILENAME}")
    message(STATUS "Copied ${FILENAME} to public/")
endforeach()
