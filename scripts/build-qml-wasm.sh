#!/bin/bash
# Build QML WASM application
# Usage: ./scripts/build-qml-wasm.sh <build-dir> <source-dir> <qt-root-dir>
#
# Parameters:
#   BUILD_DIR    - Directory for CMake build output (e.g., "build-qml")
#   SOURCE_DIR   - Source directory (e.g., "." or "$GITHUB_WORKSPACE")
#   QT_ROOT_DIR  - Path to Qt WASM installation (e.g., ~/Qt/6.6.2/wasm_multithread)

set -e

BUILD_DIR="${1:-build-qml}"
SOURCE_DIR="${2:-.}"
QT_ROOT_DIR="${3:-$QT_ROOT_DIR}"

echo "=== Building QML WASM Application ==="
echo "Build directory: $BUILD_DIR"
echo "Source directory: $SOURCE_DIR"
echo "Qt root directory: $QT_ROOT_DIR"
echo ""

# Verify Qt installation
if [ -z "$QT_ROOT_DIR" ]; then
    echo "Error: QT_ROOT_DIR not specified and not set in environment"
    exit 1
fi

if [ ! -x "$QT_ROOT_DIR/bin/qt-cmake" ]; then
    echo "Error: qt-cmake not found at $QT_ROOT_DIR/bin/qt-cmake"
    exit 1
fi

echo "Configuring CMake with qt-cmake..."
"$QT_ROOT_DIR/bin/qt-cmake" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DWISDOM_CHESS_QML_UI=ON \
    -DWISDOM_CHESS_CONSOLE_UI=OFF \
    -DWISDOM_CHESS_REACT_UI=OFF \
    -DWISDOM_CHESS_FAST_TESTS=OFF \
    -DWISDOM_CHESS_SLOW_TESTS=OFF \
    -S "$SOURCE_DIR"

echo ""
echo "Building WisdomChessQml target..."
cmake --build "$BUILD_DIR" --target WisdomChessQml -j 4

echo ""
echo "=== QML WASM build complete ==="
echo "Build output in: $BUILD_DIR/src/wisdom-chess/ui/qml/"
ls -la "$BUILD_DIR/src/wisdom-chess/ui/qml/hashed/" 2>/dev/null || echo "(hashed directory not found)"
