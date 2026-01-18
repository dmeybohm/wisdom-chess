set(cxxCompilerPath "${CMAKE_CXX_COMPILER}")
cmake_path(GET cxxCompilerPath PARENT_PATH emsdkPath)
cmake_path(SET emsdkIdlPath "${emsdkPath}/tools/webidl_binder.py")
cmake_path(APPEND CMAKE_CURRENT_SOURCE_DIR .. react/public OUTPUT_VARIABLE wisdomChessReactPublicPath)

message("-- cxxCompilerPath=" ${cxxCompilerPath})
message("-- emsdkPath=" ${emsdkPath})
message("-- emsdkIdlPath=" ${emsdkIdlPath})

find_program(PYTHON3
    NAMES
        python3 python
    )

# Generate the API for calling. This generates it by default in "glue.cpp". This moves
# this file to "glue.hpp" so that we can add it as a normal header file dependency instead of
# including a cpp file in another one and messing up its dependency generation or trying
# to link that file twice.
add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/wisdom-chess-wasm-glue.hpp
        ${CMAKE_CURRENT_BINARY_DIR}/glue.js
    DEPENDS
        wisdom-chess.idl
    COMMAND "${PYTHON3}" "${emsdkIdlPath}" ${CMAKE_CURRENT_SOURCE_DIR}/wisdom-chess.idl ${CMAKE_CURRENT_BINARY_DIR}/glue
        && mv ${CMAKE_CURRENT_BINARY_DIR}/glue.cpp ${CMAKE_CURRENT_BINARY_DIR}/wisdom-chess-wasm-glue.hpp
)

add_executable(wisdom-chess-web
        main.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/wisdom-chess-wasm-glue.hpp
        web_logger.cpp
        web_game.cpp
        bindings.cpp
        webidl_glue_wrapper.cpp
        game_model.cpp)

# Make sure the target dependencies are included:
set_target_properties(wisdom-chess-web PROPERTIES
    LINK_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/glue.js
)

target_link_libraries(wisdom-chess-web PRIVATE wisdom::chess)
target_include_directories(wisdom-chess-web
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE   ${CMAKE_CURRENT_BINARY_DIR}
                  ../../..
)
target_compile_options(wisdom-chess-web PRIVATE
        $<$<CONFIG:Debug>:-sNO_DISABLE_EXCEPTION_CATCHING>
        "-sWASM_WORKERS"
)
target_link_options(wisdom-chess-web PRIVATE
        "SHELL:-s WASM_WORKERS"
        "SHELL:-s MODULARIZE=1"
        "SHELL:-s INITIAL_MEMORY=33554432"
        "SHELL:-s MAXIMUM_MEMORY=1073741824"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        $<$<CONFIG:Debug>:-sASSERTIONS=2>
        $<$<CONFIG:Debug>:-sNO_DISABLE_EXCEPTION_CATCHING>
        "SHELL:-s EXPORT_NAME='wisdomChessWeb'"
        "SHELL:-s EXPORTED_FUNCTIONS=['_malloc, _free, _main']"
        --post-js ${CMAKE_CURRENT_BINARY_DIR}/glue.js
)

add_custom_command(TARGET wisdom-chess-web
        POST_BUILD
        COMMAND true
        BYPRODUCTS
            wisdom-chess-web.wasm
            wisdom-chess-web.ww.js
)

set(WASM_HASHED_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/hashed")

add_custom_command(TARGET wisdom-chess-web
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            -DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
            -DTEMPLATE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/wisdom-chess-load.js.in
            -DOUTPUT_DIR=${WASM_HASHED_OUTPUT_DIR}
            -P ${CMAKE_SOURCE_DIR}/cmake/ProcessWasmFiles.cmake
        COMMENT "Processing WASM files with cache-busting hashes"
)
