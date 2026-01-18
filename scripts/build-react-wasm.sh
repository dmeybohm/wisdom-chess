#!/bin/bash
# Build React WASM library
# Usage: ./scripts/build-react-wasm.sh <build-dir> <source-dir> <build-type>
#
# Parameters:
#   BUILD_DIR   - Directory for CMake build output (e.g., "build")
#   SOURCE_DIR  - Source directory (e.g., "." or "$GITHUB_WORKSPACE")
#   BUILD_TYPE  - CMake build type (e.g., "Release", "RelWithDebInfo")

set -e

BUILD_DIR="${1:-build}"
SOURCE_DIR="${2:-.}"
BUILD_TYPE="${3:-RelWithDebInfo}"

echo "=== Building React WASM Library ==="
echo "Build directory: $BUILD_DIR"
echo "Source directory: $SOURCE_DIR"
echo "Build type: $BUILD_TYPE"
echo ""

# Verify emscripten is available
if ! command -v emcmake &> /dev/null; then
    echo "Error: emcmake not found. Please source emsdk_env.sh first."
    exit 1
fi

echo "Configuring CMake..."
emcmake cmake -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DWISDOM_CHESS_FAST_TESTS=Off \
    -DWISDOM_CHESS_SLOW_TESTS=Off \
    -DWISDOM_CHESS_QML_UI=Off \
    -S "$SOURCE_DIR"

echo ""
echo "Building..."
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -j 4

echo ""
echo "=== React WASM build complete ==="
echo "Build output in: $BUILD_DIR/src/wisdom-chess/ui/wasm/"
ls -la "$BUILD_DIR/src/wisdom-chess/ui/wasm/hashed/" 2>/dev/null || echo "(hashed directory not found)"
