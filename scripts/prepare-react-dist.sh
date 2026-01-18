#!/bin/bash
# Copy hashed WASM files from build to React dist and update loader version
# Usage: ./scripts/prepare-react-dist.sh <build-dir> <react-dir>
#
# Parameters:
#   BUILD_DIR  - CMake build directory containing WASM output
#   REACT_DIR  - React project directory (contains dist/)

set -e

BUILD_DIR="${1:-build}"
REACT_DIR="${2:-src/wisdom-chess/ui/react}"

HASHED_DIR="$BUILD_DIR/src/wisdom-chess/ui/wasm/hashed"
DIST_DIR="$REACT_DIR/dist"

echo "=== Preparing React Distribution ==="
echo "Build directory: $BUILD_DIR"
echo "React directory: $REACT_DIR"
echo "Hashed files source: $HASHED_DIR"
echo "Distribution target: $DIST_DIR"
echo ""

# Verify directories exist
if [ ! -d "$HASHED_DIR" ]; then
    echo "Error: Hashed WASM directory not found: $HASHED_DIR"
    exit 1
fi

if [ ! -d "$DIST_DIR" ]; then
    echo "Error: React dist directory not found: $DIST_DIR"
    echo "Did you run 'npm run build' first?"
    exit 1
fi

# Copy hashed WASM files
echo "Copying hashed WASM files..."
for file in "$HASHED_DIR"/*.wasm; do
    if [ -f "$file" ]; then
        echo "  $(basename "$file")"
        cp "$file" "$DIST_DIR/"
    fi
done

for file in "$HASHED_DIR"/*.js; do
    if [ -f "$file" ]; then
        echo "  $(basename "$file")"
        cp "$file" "$DIST_DIR/"
    fi
done

echo ""

# Update loader version in index.html
if [ ! -f "$DIST_DIR/wisdom-chess-load.js" ]; then
    echo "Error: $DIST_DIR/wisdom-chess-load.js not found"
    exit 1
fi

if [ ! -f "$DIST_DIR/index.html" ]; then
    echo "Error: $DIST_DIR/index.html not found"
    exit 1
fi

LOADER_HASH=$(md5sum "$DIST_DIR/wisdom-chess-load.js" | cut -c1-8)
sed -i "s/wisdom-chess-load\.js?v=dev/wisdom-chess-load.js?v=$LOADER_HASH/" "$DIST_DIR/index.html"

echo "Updated loader version to: $LOADER_HASH"
echo ""

echo "=== React distribution ready ==="
echo "Files in $DIST_DIR:"
ls -la "$DIST_DIR"
