#!/bin/bash
# Update the loader version query parameter in index.html
# Usage: ./scripts/update-loader-version.sh <react-dir>

set -e

REACT_DIR="${1:-.}"

if [ ! -f "$REACT_DIR/dist/wisdom-chess-load.js" ]; then
    echo "Error: $REACT_DIR/dist/wisdom-chess-load.js not found"
    exit 1
fi

if [ ! -f "$REACT_DIR/dist/index.html" ]; then
    echo "Error: $REACT_DIR/dist/index.html not found"
    exit 1
fi

LOADER_HASH=$(md5sum "$REACT_DIR/dist/wisdom-chess-load.js" | cut -c1-8)
sed -i "s/wisdom-chess-load\.js?v=dev/wisdom-chess-load.js?v=$LOADER_HASH/" "$REACT_DIR/dist/index.html"

echo "Updated loader version to $LOADER_HASH"
