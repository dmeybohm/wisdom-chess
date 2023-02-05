set(cxxCompilerPath "${CMAKE_CXX_COMPILER}")
cmake_path(GET cxxCompilerPath PARENT_PATH emsdkPath)
cmake_path(SET emsdkIdlPath "${emsdkPath}/tools/webidl_binder.py")

message("-- cxxCompilerPath=" ${cxxCompilerPath})
message("-- emsdkPath=" ${emsdkPath})
message("-- emsdkIdlPath=" ${emsdkIdlPath})

# Generate the API for calling. This generates it by default in "glue.cpp". This moves
# this file to "glue.hpp" so that we can add it as a normal header file dependency instead of
# including a cpp file in another one and messing up its dependency generation or trying
# to link that file twice.
add_custom_command(
    OUTPUT 
        ${CMAKE_CURRENT_BINARY_DIR}/glue.hpp 
        ${CMAKE_CURRENT_BINARY_DIR}/glue.js 
        ${CMAKE_CURRENT_BINARY_DIR}/wisdom-chess-load.js
    DEPENDS 
        wisdom-chess.idl 
        wisdom-chess-load.js
    COMMAND python3 "${emsdkIdlPath}" ${CMAKE_CURRENT_SOURCE_DIR}/wisdom-chess.idl ${CMAKE_CURRENT_BINARY_DIR}/glue
        && mv ${CMAKE_CURRENT_BINARY_DIR}/glue.cpp ${CMAKE_CURRENT_BINARY_DIR}/glue.hpp
        && cp ${CMAKE_CURRENT_SOURCE_DIR}/wisdom-chess-load.js ${CMAKE_CURRENT_BINARY_DIR}/wisdom-chess-load.js
)

add_executable(wisdom-chess-web empty_main.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/glue.hpp
        webidl_glue_wrapper.cpp
        )

# Make sure the target dependencies are included:
set_target_properties(wisdom-chess-web PROPERTIES
        LINK_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/wisdom-chess-load.js
        LINK_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/glue.js
        )

target_link_libraries(wisdom-chess-web PRIVATE wisdom::chess)
target_include_directories(wisdom-chess-web
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE   ${CMAKE_CURRENT_BINARY_DIR}
        )
target_link_options(wisdom-chess-web PRIVATE
    "SHELL:-s MODULARIZE=1" "SHELL:-s EXPORT_NAME='wisdomChessWeb'" --post-js ${CMAKE_CURRENT_BINARY_DIR}/glue.js
)

# Ensure "make clean" gets rid of the wasm file:
# (There is an ADDITIONAL_CLEAN_FILES on the target properties but that doesn't work)
add_custom_command(
    TARGET wisdom-chess-web
    COMMAND true
    POST_BUILD
    BYPRODUCTS wisdom-chess-web.wasm 
)
