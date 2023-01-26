set(cxxCompilerPath "${CMAKE_CXX_COMPILER}")
cmake_path(GET cxxCompilerPath PARENT_PATH emsdkPath)
cmake_path(SET emsdkIdlPath "${emsdkPath}/tools/webidl_binder.py")

message("-- cxxCompilerPath=" ${cxxCompilerPath})
message("-- emsdkPath=" ${emsdkPath})
message("-- emsdkIdlPath=" ${emsdkIdlPath})

# Generate the API for calling
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/glue.js ${CMAKE_CURRENT_BINARY_DIR}/glue.cpp 
    DEPENDS wisdomchess.idl webidl_glue_wrapper.cpp
    COMMAND python3 "${emsdkIdlPath}" ${CMAKE_CURRENT_SOURCE_DIR}/wisdomchess.idl ${CMAKE_CURRENT_BINARY_DIR}/glue
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test.js
    DEPENDS test-src.js wisdomchess.idl webidl_glue_wrapper.cpp
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/test-src.js ${CMAKE_CURRENT_BINARY_DIR}/test.js
)

add_executable(wasm-cmake empty_main.cpp webidl_glue_wrapper.cpp test.js)
target_link_libraries(wasm-cmake PRIVATE wisdom::chess)
target_include_directories(wasm-cmake PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_options(wasm-cmake PRIVATE
    "SHELL:-s MODULARIZE=1" "SHELL:-s EXPORT_NAME='wasmCmake'" --post-js ${CMAKE_CURRENT_BINARY_DIR}/glue.js
)

# Ensure "make clean" gets rid of the wasm file:
add_custom_command(
    TARGET wasm-cmake
    COMMAND echo
    POST_BUILD
    BYPRODUCTS wasm-cmake.wasm
)
