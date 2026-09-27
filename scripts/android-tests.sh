#!/bin/bash
# Build for Android and run the tests on the connected device or emulator.
# Used by .github/workflows/cmake.yml, inside the step that runs the
# emulator, and usable locally.
#
# Usage: ./scripts/android-tests.sh <qt-android-dir> [build-dir]
#
# Parameters:
#   QT_ANDROID_DIR - The Qt built for the device's architecture, such as
#                    ~/Qt/6.9.3/android_x86_64 for an emulator
#   BUILD_DIR      - CMake build directory (default: build-android-tests)
#
# Environment:
#   JAVA_HOME         - A Java 17
#   ANDROID_SDK_ROOT  - The Android SDK (default: $ANDROID_HOME)
#   ANDROID_NDK_ROOT  - The Android NDK
#   QT_HOST_PATH      - The desktop Qt (default: gcc_64 beside QT_ANDROID_DIR)
#   CPM_SOURCE_CACHE  - Where CPM keeps its downloads (default: none)
#   ANDROID_SERIAL    - The device, when several are connected
#
# The build runs the test programs to list their tests, so the device has to
# be there from the start.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="$SCRIPT_DIR/.."

QT_ANDROID_DIR="$1"
BUILD_DIR="${2:-build-android-tests}"

fail() {
    echo "Error: $*" >&2
    exit 1
}

[ -n "$QT_ANDROID_DIR" ] || fail "usage: $0 <qt-android-dir> [build-dir]"
[ -x "$QT_ANDROID_DIR/bin/qt-cmake" ] || fail "no bin/qt-cmake in $QT_ANDROID_DIR"

ANDROID_SDK_ROOT="${ANDROID_SDK_ROOT:-$ANDROID_HOME}"
QT_HOST_PATH="${QT_HOST_PATH:-$(dirname "$QT_ANDROID_DIR")/gcc_64}"

[ -d "$ANDROID_SDK_ROOT" ] || fail "ANDROID_SDK_ROOT is not set to a directory"
[ -d "$ANDROID_NDK_ROOT" ] || fail "ANDROID_NDK_ROOT is not set to a directory"
[ -d "$QT_HOST_PATH" ] || fail "no desktop Qt at $QT_HOST_PATH; set QT_HOST_PATH"

echo "=== Android tests ==="
echo "Qt for Android: $QT_ANDROID_DIR"
echo "Desktop Qt: $QT_HOST_PATH"
echo "SDK: $ANDROID_SDK_ROOT"
echo "NDK: $ANDROID_NDK_ROOT"
echo "Build directory: $BUILD_DIR"
echo ""

cmake_args=(
    -S "$SOURCE_DIR" -B "$BUILD_DIR" -G Ninja
    -DCMAKE_BUILD_TYPE=Release
    "-DQT_HOST_PATH=$QT_HOST_PATH"
    "-DANDROID_SDK_ROOT=$ANDROID_SDK_ROOT"
    "-DANDROID_NDK_ROOT=$ANDROID_NDK_ROOT"
    -DWISDOM_CHESS_QML_UI=On
    -DWISDOM_CHESS_BUILD_LINTER=Off
    -DWISDOM_CHESS_FAST_TESTS=On
    -DWISDOM_CHESS_SLOW_TESTS=On
)
if [ -n "$CPM_SOURCE_CACHE" ]; then
    cmake_args+=("-DCPM_SOURCE_CACHE=$CPM_SOURCE_CACHE")
fi

"$QT_ANDROID_DIR/bin/qt-cmake" "${cmake_args[@]}"
cmake --build "$BUILD_DIR" -j 4

# A test gets a second try: now and then one passes on the device and its
# result is lost on the way back.
ctest --test-dir "$BUILD_DIR" --output-on-failure -j 4 --repeat until-pass:2
