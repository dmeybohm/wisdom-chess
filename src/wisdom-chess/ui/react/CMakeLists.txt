# Node.js detection (when integrated build is enabled)
if(WISDOM_CHESS_REACT_BUILD_INTEGRATED)
    find_program(NODE_EXECUTABLE NAMES node nodejs)
    find_program(NPM_EXECUTABLE NAMES npm)

    if(NOT NODE_EXECUTABLE)
        message(FATAL_ERROR "Node.js executable not found. Please install Node.js or set NODE_EXECUTABLE.")
    endif()

    if(NOT NPM_EXECUTABLE)
        message(FATAL_ERROR "npm executable not found. Please install npm or set NPM_EXECUTABLE.")
    endif()

    message(STATUS "Found Node.js: ${NODE_EXECUTABLE}")
    message(STATUS "Found npm: ${NPM_EXECUTABLE}")

    # Check Node.js version
    execute_process(
        COMMAND ${NODE_EXECUTABLE} --version
        OUTPUT_VARIABLE NODE_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "Node.js version: ${NODE_VERSION}")
endif()

# React build targets (conditional on integrated build)
if(WISDOM_CHESS_REACT_BUILD_INTEGRATED)
    # Custom target for npm install
    add_custom_target(
        wisdom-chess-react-install
        COMMAND ${NPM_EXECUTABLE} install
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Installing React dependencies"
    )

    # Custom target for npm build
    add_custom_target(
        wisdom-chess-react-build
        COMMAND ${NPM_EXECUTABLE} run build
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Building React frontend"
        DEPENDS wisdom-chess-react-install wisdom-chess-react-public-dev
    )

    # Custom target for npm dev server
    add_custom_target(
        wisdom-chess-react-dev
        COMMAND ${NPM_EXECUTABLE} run dev
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Starting React development server"
        DEPENDS wisdom-chess-react-install wisdom-chess-react-public-dev
    )

    # Make the build target part of ALL if integrated build is enabled
    add_custom_target(
        wisdom-chess-react ALL
        DEPENDS wisdom-chess-react-build
    )
else()
    # Dummy target when integrated build is disabled
    add_custom_target(
        wisdom-chess-react
        COMMAND ${CMAKE_COMMAND} -E echo "React integrated build is disabled. Enable with -DWISDOM_CHESS_REACT_BUILD_INTEGRATED=ON"
    )
endif()

# Setup the wasm files in react/public for development.
# Uses a marker file since hashed filenames are dynamic.
add_custom_command(
    COMMENT "Updating files in react/public for development"
    DEPENDS wisdom-chess-web
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/public/.wasm-files-copied
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/../wasm/hashed/wisdom-chess-load.js
        ${CMAKE_CURRENT_SOURCE_DIR}/public/wisdom-chess-load.js
    COMMAND ${CMAKE_COMMAND}
        -DSOURCE_DIR=${CMAKE_CURRENT_BINARY_DIR}/../wasm/hashed
        -DDEST_DIR=${CMAKE_CURRENT_SOURCE_DIR}/public
        -P ${CMAKE_SOURCE_DIR}/cmake/CopyHashedFiles.cmake
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_BINARY_DIR}/../wasm/glue.js
        ${CMAKE_CURRENT_SOURCE_DIR}/public/glue.js
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_SOURCE_DIR}/public/.wasm-files-copied
)

add_custom_target(
    wisdom-chess-react-public-dev
    ALL
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/public/.wasm-files-copied
)

# Install target for production build output
if(WISDOM_CHESS_REACT_BUILD_INTEGRATED)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dist/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/wisdom-chess/web
        OPTIONAL  # Only install if dist directory exists
    )
endif()
