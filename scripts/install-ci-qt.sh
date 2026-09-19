#!/bin/bash
# Install the Qt that CI builds against, so a CI-only failure can be
# reproduced locally. Nothing is installed system-wide and no root is
# needed: aqtinstall goes into a virtual environment and Qt into a cache
# directory.
#
# Usage: ./scripts/install-ci-qt.sh [version] [directory]
#
# Parameters:
#   VERSION    - Qt version to install (default: the newest release matching
#                the version .github/workflows/cmake.yml asks for)
#   DIRECTORY  - where Qt and the aqtinstall environment go
#                (default: $XDG_CACHE_HOME/wisdom-chess/qt, or
#                $HOME/.cache/wisdom-chess/qt)
#
# Both can also be given as WISDOM_CHESS_QT_VERSION and
# WISDOM_CHESS_QT_CACHE. The path to pass to CMake is printed at the end,
# whether or not anything was installed.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKFLOW="$SCRIPT_DIR/../.github/workflows/cmake.yml"

VERSION="${1:-${WISDOM_CHESS_QT_VERSION:-}}"
QT_CACHE="${2:-${WISDOM_CHESS_QT_CACHE:-${XDG_CACHE_HOME:-$HOME/.cache}/wisdom-chess/qt}}"

VENV_DIR="$QT_CACHE/aqt-venv"
QT_DIR="$QT_CACHE/Qt"
AQT="$VENV_DIR/bin/aqt"

fail() {
    echo "Error: $*" >&2
    exit 1
}

# The version CI asks install-qt-action for, such as 6.9.*.
ci_version() {
    local spec
    spec="$(sed -n "s/^[[:space:]]*version:[[:space:]]*['\"]\\(.*\\)['\"][[:space:]]*$/\\1/p" "$WORKFLOW" | head -1)"
    [ -n "$spec" ] || fail "no Qt version found in $WORKFLOW"
    echo "$spec"
}

case "$(uname -s)" in
    Linux)
        AQT_TARGET=(linux desktop)
        AQT_ARCH=linux_gcc_64
        QT_ARCH_DIR=gcc_64
        ;;
    *)
        fail "only Linux is supported; on $(uname -s), install the Qt that" \
             ".github/workflows/cmake.yml asks for by hand"
        ;;
esac

echo "=== Installing the Qt used by CI ==="

[ -f "$WORKFLOW" ] || fail "workflow not found: $WORKFLOW"

command -v python3 > /dev/null || fail "python3 is not installed"
python3 -c 'import venv' 2> /dev/null \
    || fail "the python3 venv module is missing (Debian and Ubuntu package it as python3-venv)"

if [ ! -x "$AQT" ]; then
    echo "Creating the aqtinstall environment in $VENV_DIR"
    mkdir -p "$QT_CACHE"
    python3 -m venv "$VENV_DIR"
    "$VENV_DIR/bin/pip" install --quiet --upgrade pip
    "$VENV_DIR/bin/pip" install --quiet aqtinstall
fi

# aqt writes aqtinstall.log to the current directory.
cd "$QT_CACHE"

if [ -z "$VERSION" ]; then
    SPEC="$(ci_version)"
    echo "CI asks for Qt $SPEC; looking up the newest matching release"

    # install-qt-action resolves 6.9.* to the newest 6.9; --spec takes the
    # same range without the wildcard.
    VERSION="$("$AQT" list-qt "${AQT_TARGET[@]}" --spec "${SPEC%.\*}" --latest-version)"
    [ -n "$VERSION" ] || fail "no Qt release matching $SPEC"
fi

INSTALLED="$QT_DIR/$VERSION/$QT_ARCH_DIR"

if [ -e "$INSTALLED/bin/qmake" ]; then
    echo "Qt $VERSION is already installed"
else
    echo "Installing Qt $VERSION ($AQT_ARCH) into $QT_DIR"
    "$AQT" install-qt "${AQT_TARGET[@]}" "$VERSION" "$AQT_ARCH" --outputdir "$QT_DIR"
    [ -e "$INSTALLED/bin/qmake" ] || fail "install finished but $INSTALLED holds no Qt"
fi

echo ""
echo "Configure against it with:"
echo ""
echo "  cmake -S . -B build-ci-qt -DCMAKE_BUILD_TYPE=Debug \\"
echo "      -DWISDOM_CHESS_QT_DIR=$INSTALLED \\"
echo "      -DWISDOM_CHESS_QML_UI=ON"
echo ""
