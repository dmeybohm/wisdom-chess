#!/bin/bash
# Run clazy static analysis on the Qt/QML source files.
#
# Usage: ./scripts/run-clazy.sh [options]
#
# Options:
#   --qt-dir <path>    Path to Qt installation (default: auto-detect)
#   --build-dir <path> Build directory (default: build-clazy)
#   --checks <checks>  Clazy checks to enable (default: level0,level1,level2)
#   --reconfigure      Force CMake reconfiguration
#   --help             Show this help message
#
# The script handles the Clang/GCC libstdc++ incompatibility by using
# clazy-standalone with explicit GCC include paths.

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SOURCE_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

BUILD_DIR="$SOURCE_DIR/build-clazy"
QT_DIR=""
CHECKS="level0,level1,level2"
RECONFIGURE=0

usage() {
    head -14 "$0" | tail -12 | sed 's/^# \?//'
    exit 0
}

while [[ $# -gt 0 ]]; do
    case $1 in
        --qt-dir)    QT_DIR="$2"; shift 2 ;;
        --build-dir) BUILD_DIR="$2"; shift 2 ;;
        --checks)    CHECKS="$2"; shift 2 ;;
        --reconfigure) RECONFIGURE=1; shift ;;
        --help)      usage ;;
        *) echo "Unknown option: $1"; usage ;;
    esac
done

# Check for required tools
for tool in clazy-standalone cmake; do
    if ! command -v "$tool" &>/dev/null; then
        echo "Error: $tool not found in PATH"
        exit 1
    fi
done

# Auto-detect Qt directory
if [ -z "$QT_DIR" ]; then
    if [ -n "$WISDOM_CHESS_QT_DIR" ]; then
        QT_DIR="$WISDOM_CHESS_QT_DIR"
    else
        # Try common locations
        for candidate in \
            "$HOME/Qt"/6.*/gcc_64 \
            /usr/lib/x86_64-linux-gnu/cmake/Qt6/..
        do
            if [ -f "$candidate/lib/cmake/Qt6Quick/Qt6QuickConfig.cmake" ] 2>/dev/null; then
                QT_DIR="$(cd "$candidate" && pwd)"
                break
            fi
        done
    fi
fi

if [ -z "$QT_DIR" ]; then
    echo "Error: Could not find Qt installation."
    echo "Set --qt-dir, WISDOM_CHESS_QT_DIR, or install Qt6 Quick."
    exit 1
fi

echo "=== Clazy Static Analysis ==="
echo "Source directory: $SOURCE_DIR"
echo "Build directory:  $BUILD_DIR"
echo "Qt directory:     $QT_DIR"
echo "Checks:           $CHECKS"
echo ""

# Detect the GCC version that clazy-standalone's Clang is compatible with.
# clazy-standalone is based on an older Clang that may not parse newer
# GCC libstdc++ headers. We find the oldest available GCC include path.
find_compatible_gcc_includes() {
    local gcc_base="/usr/lib/gcc/x86_64-linux-gnu"
    local gcc_version=""

    # Pick the oldest GCC version available (most compatible with older Clang)
    for dir in "$gcc_base"/*/; do
        local ver
        ver="$(basename "$dir")"
        if [ -z "$gcc_version" ] || [ "$(printf '%s\n%s' "$ver" "$gcc_version" | sort -V | head -1)" = "$ver" ]; then
            gcc_version="$ver"
        fi
    done

    if [ -z "$gcc_version" ]; then
        echo ""
        return
    fi

    local include_base="$gcc_base/$gcc_version/../../../../include/c++/$gcc_version"
    if [ -d "$include_base" ]; then
        echo "$include_base"
    fi
}

GCC_INCLUDE="$(find_compatible_gcc_includes)"

# Configure CMake if needed
if [ ! -f "$BUILD_DIR/compile_commands.json" ] || [ "$RECONFIGURE" -eq 1 ]; then
    echo "--- Configuring CMake ---"
    cmake -B "$BUILD_DIR" -S "$SOURCE_DIR" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_PREFIX_PATH="$QT_DIR" \
        -DWISDOM_CHESS_QT_DIR="$QT_DIR" \
        -DWISDOM_CHESS_QML_UI=On \
        -DWISDOM_CHESS_FAST_TESTS=Off \
        -DWISDOM_CHESS_SLOW_TESTS=Off \
        -DWISDOM_CHESS_BUILD_LINTER=Off \
        -Wno-dev
    echo ""
fi

# Build the QML target (needed for MOC-generated files)
echo "--- Building QML target ---"
cmake --build "$BUILD_DIR" --target WisdomChessQml -j "$(nproc)" 2>&1
echo ""

# Collect QML C++ source files
QML_SOURCES=()
while IFS= read -r -d '' file; do
    QML_SOURCES+=("$file")
done < <(find "$SOURCE_DIR/src/wisdom-chess/ui/qml/main" -name "*.cpp" -print0 | sort -z)

if [ ${#QML_SOURCES[@]} -eq 0 ]; then
    echo "Error: No QML source files found"
    exit 1
fi

echo "--- Running clazy-standalone on ${#QML_SOURCES[@]} files ---"

# Build extra args for GCC header compatibility
EXTRA_ARGS=()
if [ -n "$GCC_INCLUDE" ]; then
    EXTRA_ARGS+=(--extra-arg="-isystem$GCC_INCLUDE")
    EXTRA_ARGS+=(--extra-arg="-isystem${GCC_INCLUDE}/x86_64-linux-gnu")
    echo "Using GCC includes: $GCC_INCLUDE"
fi
echo ""

clazy-standalone \
    -checks="$CHECKS" \
    -p "$BUILD_DIR/compile_commands.json" \
    "${EXTRA_ARGS[@]}" \
    "${QML_SOURCES[@]}" \
    2>&1

echo ""
echo "=== Clazy analysis complete ==="
