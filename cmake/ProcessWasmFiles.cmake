# ProcessWasmFiles.cmake - Process WASM files with cache-busting hashes
#
# Usage:
#   cmake -DSOURCE_DIR=<dir> -DBINARY_DIR=<dir> -DTEMPLATE_FILE=<file> -DOUTPUT_DIR=<dir> -P ProcessWasmFiles.cmake
#
# This script:
#   1. Computes MD5 hashes of WASM build outputs
#   2. Copies files with hashed names to OUTPUT_DIR
#   3. Generates wisdom-chess-load.js from template with correct filenames

if(NOT CMAKE_SCRIPT_MODE_FILE)
    message(FATAL_ERROR "This script must be run with cmake -P")
endif()

if(NOT DEFINED BINARY_DIR)
    message(FATAL_ERROR "BINARY_DIR must be defined")
endif()
if(NOT DEFINED TEMPLATE_FILE)
    message(FATAL_ERROR "TEMPLATE_FILE must be defined")
endif()
if(NOT DEFINED OUTPUT_DIR)
    message(FATAL_ERROR "OUTPUT_DIR must be defined")
endif()

function(hash_and_copy INPUT_FILE OUTPUT_DIR OUT_HASHED_NAME)
    if(NOT EXISTS "${INPUT_FILE}")
        message(FATAL_ERROR "Input file does not exist: ${INPUT_FILE}")
    endif()

    file(MD5 "${INPUT_FILE}" FULL_HASH)
    string(SUBSTRING "${FULL_HASH}" 0 8 SHORT_HASH)

    get_filename_component(NAME_WE "${INPUT_FILE}" NAME_WE)
    get_filename_component(EXT "${INPUT_FILE}" EXT)
    set(HASHED_NAME "${NAME_WE}.${SHORT_HASH}${EXT}")

    file(MAKE_DIRECTORY "${OUTPUT_DIR}")
    file(COPY_FILE "${INPUT_FILE}" "${OUTPUT_DIR}/${HASHED_NAME}")

    message(STATUS "Cache-busted: ${INPUT_FILE} -> ${HASHED_NAME}")
    set(${OUT_HASHED_NAME} "${HASHED_NAME}" PARENT_SCOPE)
endfunction()

file(MAKE_DIRECTORY "${OUTPUT_DIR}")

hash_and_copy("${BINARY_DIR}/wisdom-chess-web.js" "${OUTPUT_DIR}" WASM_JS_FILE)
hash_and_copy("${BINARY_DIR}/wisdom-chess-web.wasm" "${OUTPUT_DIR}" WASM_FILE)
hash_and_copy("${BINARY_DIR}/wisdom-chess-web.ww.js" "${OUTPUT_DIR}" WASM_WORKER_FILE)

configure_file("${TEMPLATE_FILE}" "${OUTPUT_DIR}/wisdom-chess-load.js" @ONLY)

file(MD5 "${OUTPUT_DIR}/wisdom-chess-load.js" LOADER_HASH)
string(SUBSTRING "${LOADER_HASH}" 0 8 LOADER_SHORT_HASH)
message(STATUS "Loader hash for query param: ${LOADER_SHORT_HASH}")

file(WRITE "${OUTPUT_DIR}/cache-bust-vars.txt"
    "WASM_JS_FILE=${WASM_JS_FILE}\n"
    "WASM_FILE=${WASM_FILE}\n"
    "WASM_WORKER_FILE=${WASM_WORKER_FILE}\n"
    "LOADER_HASH=${LOADER_SHORT_HASH}\n"
)
