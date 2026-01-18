#!/bin/bash
# Combine React dist and QML build into deployment directory
# Usage: ./scripts/prepare-deployment.sh <react-dist-dir> <qml-build-dir> <qml-source-dir> <deploy-dir>
#
# Parameters:
#   REACT_DIST_DIR  - React distribution directory (e.g., "src/wisdom-chess/ui/react/dist")
#   QML_BUILD_DIR   - QML CMake build directory (e.g., "build-qml")
#   QML_SOURCE_DIR  - QML source directory (e.g., "src/wisdom-chess/ui/qml")
#   DEPLOY_DIR      - Output deployment directory (e.g., "deploy")

set -e

REACT_DIST_DIR="${1:-src/wisdom-chess/ui/react/dist}"
QML_BUILD_DIR="${2:-build-qml}"
QML_SOURCE_DIR="${3:-src/wisdom-chess/ui/qml}"
DEPLOY_DIR="${4:-deploy}"

QML_HASHED_DIR="$QML_BUILD_DIR/src/wisdom-chess/ui/qml/hashed"
QML_WASM_ASSETS="$QML_SOURCE_DIR/wasm"

echo "=== Preparing Combined Deployment ==="
echo "React dist directory: $REACT_DIST_DIR"
echo "QML build directory: $QML_BUILD_DIR"
echo "QML source directory: $QML_SOURCE_DIR"
echo "Deploy directory: $DEPLOY_DIR"
echo ""

# Verify source directories
if [ ! -d "$REACT_DIST_DIR" ]; then
    echo "Error: React dist directory not found: $REACT_DIST_DIR"
    exit 1
fi

if [ ! -d "$QML_HASHED_DIR" ]; then
    echo "Error: QML hashed directory not found: $QML_HASHED_DIR"
    exit 1
fi

# Create deployment directory
echo "Creating deployment directory..."
mkdir -p "$DEPLOY_DIR"
mkdir -p "$DEPLOY_DIR/qml"

# Copy React dist as the base
echo ""
echo "Copying React distribution..."
cp -r "$REACT_DIST_DIR"/* "$DEPLOY_DIR/"

# Copy QML build artifacts from hashed/ directory
echo ""
echo "Copying QML build artifacts..."
echo "  WisdomChessQml.wasm"
cp "$QML_HASHED_DIR/WisdomChessQml.wasm" "$DEPLOY_DIR/qml/"
echo "  WisdomChessQml.js"
cp "$QML_HASHED_DIR/WisdomChessQml.js" "$DEPLOY_DIR/qml/"
echo "  qtloader.js"
cp "$QML_HASHED_DIR/qtloader.js" "$DEPLOY_DIR/qml/"
echo "  index.html (with cache-busting query params)"
cp "$QML_HASHED_DIR/index.html" "$DEPLOY_DIR/qml/"

# Copy QML static assets (icons, etc.) but NOT index.html or netlify.toml
echo ""
echo "Copying QML static assets..."
for ext in svg webmanifest png ico; do
    for file in "$QML_WASM_ASSETS"/*.$ext; do
        if [ -f "$file" ]; then
            echo "  $(basename "$file")"
            cp "$file" "$DEPLOY_DIR/qml/"
        fi
    done
done

# Remove QML's netlify.toml if present (use React's)
rm -f "$DEPLOY_DIR/qml/netlify.toml"

echo ""
echo "=== Deployment directory ready ==="
echo ""
echo "Root directory contents:"
ls -la "$DEPLOY_DIR"
echo ""
echo "QML subdirectory contents:"
ls -la "$DEPLOY_DIR/qml"
