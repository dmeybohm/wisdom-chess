# ProcessQmlWasmFiles.cmake - Process QML WASM files for cache-busting
#
# Usage:
#   cmake -DBINARY_DIR=<dir> -DTEMPLATE_FILE=<file> -DOUTPUT_DIR=<dir> -P ProcessQmlWasmFiles.cmake
#
# This script:
#   1. Computes MD5 hashes of QML WASM build outputs
#   2. Copies files (with original names) to OUTPUT_DIR
#   3. Generates index.html from template with query param hashes
#
# Note: Unlike React, we can't rename QML WASM files because Qt's generated
# JavaScript loads the WASM file by its original name. Instead, we use
# query parameters for cache busting.

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

function(compute_hash_and_copy INPUT_FILE OUTPUT_DIR OUT_HASH)
    if(NOT EXISTS "${INPUT_FILE}")
        message(FATAL_ERROR "Input file does not exist: ${INPUT_FILE}")
    endif()

    file(MD5 "${INPUT_FILE}" FULL_HASH)
    string(SUBSTRING "${FULL_HASH}" 0 8 SHORT_HASH)

    get_filename_component(FILENAME "${INPUT_FILE}" NAME)

    file(MAKE_DIRECTORY "${OUTPUT_DIR}")
    file(COPY_FILE "${INPUT_FILE}" "${OUTPUT_DIR}/${FILENAME}")

    message(STATUS "Copied ${FILENAME} (hash: ${SHORT_HASH})")
    set(${OUT_HASH} "${SHORT_HASH}" PARENT_SCOPE)
endfunction()

file(MAKE_DIRECTORY "${OUTPUT_DIR}")

compute_hash_and_copy("${BINARY_DIR}/WisdomChessQml.js" "${OUTPUT_DIR}" QML_JS_HASH)
compute_hash_and_copy("${BINARY_DIR}/WisdomChessQml.wasm" "${OUTPUT_DIR}" QML_WASM_HASH)
compute_hash_and_copy("${BINARY_DIR}/qtloader.js" "${OUTPUT_DIR}" QTLOADER_HASH)

configure_file("${TEMPLATE_FILE}" "${OUTPUT_DIR}/index.html" @ONLY)

message(STATUS "Generated index.html with query params:")
message(STATUS "  QTLOADER_HASH: ${QTLOADER_HASH}")
message(STATUS "  QML_JS_HASH: ${QML_JS_HASH}")
message(STATUS "  (WASM hash: ${QML_WASM_HASH})")

file(WRITE "${OUTPUT_DIR}/cache-bust-vars.txt"
    "QML_JS_HASH=${QML_JS_HASH}\n"
    "QML_WASM_HASH=${QML_WASM_HASH}\n"
    "QTLOADER_HASH=${QTLOADER_HASH}\n"
)
