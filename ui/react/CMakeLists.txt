add_custom_target(
    wisdom-chess-react-public-dev
    ALL
    SOURCES
        public/wisdom-chess-web.wasm
        public/wisdom-chess-load.js
        public/wisdom-chess-web.js
        public/wisdom-chess-web.ww.js
        public/glue.js
)

add_dependencies(wisdom-chess-react-public-dev
    wisdom-chess-web
)

# Setup the wasm files in react/public for development:
add_custom_command(
    COMMENT
        "Updating files in react/public for development"
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/../wasm/wisdom-chess-web.wasm
        ${CMAKE_CURRENT_BINARY_DIR}/../wasm/wisdom-chess-web.js
        ${CMAKE_CURRENT_BINARY_DIR}/../wasm/wisdom-chess-load.js
        ${CMAKE_CURRENT_BINARY_DIR}/../wasm/wisdom-chess-web.ww.js
        ${CMAKE_CURRENT_BINARY_DIR}/../wasm/glue.js
    OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/public/wisdom-chess-web.wasm
        ${CMAKE_CURRENT_SOURCE_DIR}/public/wisdom-chess-web.js
        ${CMAKE_CURRENT_SOURCE_DIR}/public/wisdom-chess-load.js
        ${CMAKE_CURRENT_SOURCE_DIR}/public/wisdom-chess-web.ww.js
        ${CMAKE_CURRENT_SOURCE_DIR}/public/glue.js
    COMMAND
        ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/../wasm/wisdom-chess-web.wasm ${CMAKE_CURRENT_SOURCE_DIR}/public/wisdom-chess-web.wasm &&
        ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/../wasm/wisdom-chess-web.js ${CMAKE_CURRENT_SOURCE_DIR}/public/wisdom-chess-web.js &&
        ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/../wasm/wisdom-chess-web.ww.js ${CMAKE_CURRENT_SOURCE_DIR}/public/wisdom-chess-web.ww.js &&
        ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/../wasm/wisdom-chess-load.js ${CMAKE_CURRENT_SOURCE_DIR}/public/wisdom-chess-load.js &&
        ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/../wasm/glue.js ${CMAKE_CURRENT_SOURCE_DIR}/public/glue.js
)
