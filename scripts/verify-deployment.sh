#!/bin/bash
# Verify deployment directory correctness
# Usage: ./scripts/verify-deployment.sh <deploy-dir>
#
# Parameters:
#   DEPLOY_DIR - Deployment directory to verify (e.g., "deploy")
#
# Checks:
#   - Loader references hashed filenames (not @WASM_FILE@ or wisdom-chess-web.wasm)
#   - All expected hashed files exist (*.*.wasm, *.*.js patterns)
#   - Required files are present (index.html, glue.js, etc.)
#   - QML files are present and index.html has cache-busting query params

set -e

DEPLOY_DIR="${1:-deploy}"
ERRORS=0

echo "=== Verifying Deployment ==="
echo "Deploy directory: $DEPLOY_DIR"
echo ""

# Verify deploy directory exists
if [ ! -d "$DEPLOY_DIR" ]; then
    echo "ERROR: Deploy directory not found: $DEPLOY_DIR"
    exit 1
fi

# === React verification ===
echo "--- React Frontend Verification ---"

# Check index.html exists
if [ ! -f "$DEPLOY_DIR/index.html" ]; then
    echo "ERROR: index.html not found"
    ERRORS=$((ERRORS + 1))
else
    echo "OK: index.html exists"

    # Check loader version is not 'dev'
    if grep -q 'wisdom-chess-load\.js?v=dev' "$DEPLOY_DIR/index.html"; then
        echo "ERROR: index.html still has dev version for loader"
        ERRORS=$((ERRORS + 1))
    else
        echo "OK: loader has production version"
    fi
fi

# Check wisdom-chess-load.js exists
if [ ! -f "$DEPLOY_DIR/wisdom-chess-load.js" ]; then
    echo "ERROR: wisdom-chess-load.js not found"
    ERRORS=$((ERRORS + 1))
else
    echo "OK: wisdom-chess-load.js exists"

    # Check loader doesn't contain unreplaced placeholders
    if grep -q '@WASM_FILE@\|@GLUE_FILE@' "$DEPLOY_DIR/wisdom-chess-load.js"; then
        echo "ERROR: wisdom-chess-load.js contains unreplaced placeholders"
        ERRORS=$((ERRORS + 1))
    else
        echo "OK: no unreplaced placeholders in loader"
    fi

    # Check loader doesn't reference unhashed filenames
    if grep -q '"wisdom-chess-web\.wasm"\|"wisdom-chess-web\.js"' "$DEPLOY_DIR/wisdom-chess-load.js"; then
        echo "ERROR: wisdom-chess-load.js references unhashed filenames"
        ERRORS=$((ERRORS + 1))
    else
        echo "OK: loader references hashed filenames"
    fi
fi

# Check for hashed WASM files (pattern: wisdom-chess-web.*.wasm)
WASM_COUNT=$(find "$DEPLOY_DIR" -maxdepth 1 -name "wisdom-chess-web.*.wasm" 2>/dev/null | wc -l)
if [ "$WASM_COUNT" -eq 0 ]; then
    echo "ERROR: No hashed WASM files found (expected wisdom-chess-web.*.wasm)"
    ERRORS=$((ERRORS + 1))
else
    echo "OK: Found $WASM_COUNT hashed WASM file(s)"
fi

# Check for hashed glue JS files (pattern: wisdom-chess-web.*.js)
GLUE_COUNT=$(find "$DEPLOY_DIR" -maxdepth 1 -name "wisdom-chess-web.*.js" 2>/dev/null | wc -l)
if [ "$GLUE_COUNT" -eq 0 ]; then
    echo "ERROR: No hashed glue JS files found (expected wisdom-chess-web.*.js)"
    ERRORS=$((ERRORS + 1))
else
    echo "OK: Found $GLUE_COUNT hashed glue JS file(s)"
fi

echo ""

# === QML verification ===
echo "--- QML Frontend Verification ---"

QML_DIR="$DEPLOY_DIR/qml"

if [ ! -d "$QML_DIR" ]; then
    echo "ERROR: QML directory not found: $QML_DIR"
    ERRORS=$((ERRORS + 1))
else
    echo "OK: QML directory exists"

    # Check required QML files
    for file in WisdomChessQml.wasm WisdomChessQml.js qtloader.js index.html; do
        if [ ! -f "$QML_DIR/$file" ]; then
            echo "ERROR: $file not found in QML directory"
            ERRORS=$((ERRORS + 1))
        else
            echo "OK: $file exists"
        fi
    done

    # Check QML index.html has cache-busting query params
    if [ -f "$QML_DIR/index.html" ]; then
        if grep -q 'WisdomChessQml\.js?v=' "$QML_DIR/index.html"; then
            echo "OK: QML index.html has cache-busting query params"
        else
            echo "WARNING: QML index.html may not have cache-busting query params"
        fi
    fi

    # Verify no netlify.toml in QML directory
    if [ -f "$QML_DIR/netlify.toml" ]; then
        echo "WARNING: netlify.toml found in QML directory (should use React's)"
    fi
fi

echo ""

# === File listing ===
echo "--- Deployment Contents ---"
echo ""
echo "Root directory:"
ls -lah "$DEPLOY_DIR" | grep -v "^d" | tail -n +2
echo ""
if [ -d "$QML_DIR" ]; then
    echo "QML directory:"
    ls -lah "$QML_DIR" | grep -v "^d" | tail -n +2
fi

echo ""

# === Summary ===
if [ $ERRORS -gt 0 ]; then
    echo "=== VERIFICATION FAILED ==="
    echo "$ERRORS error(s) found"
    exit 1
else
    echo "=== VERIFICATION PASSED ==="
    echo "Deployment is ready"
    exit 0
fi
