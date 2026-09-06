#!/bin/bash
# Install a freshly built Qt Installer Framework installer headlessly and
# check the installed tree. Used by .github/workflows/installers.yml and
# usable locally.
#
# Usage: ./scripts/smoke-test-installer.sh <build-dir> <install-root>
#
# Parameters:
#   BUILD_DIR     - CMake build directory containing wisdom-chess-*.run/.exe/.dmg
#                   (default: build-installer)
#   INSTALL_ROOT  - Directory to install into; removed again at the end
#                   (default: $HOME/wisdom-chess-smoke)
#
# On Linux the root should live under $HOME so the installer takes the
# per-user .desktop branch and does not need elevation.

set -e

BUILD_DIR="${1:-build-installer}"
INSTALL_ROOT="${2:-$HOME/wisdom-chess-smoke}"

IFW_ARGS=(--platform minimal --root "$INSTALL_ROOT" --accept-licenses --confirm-command install)

echo "=== Smoke testing the installer ==="
echo "Build directory: $BUILD_DIR"
echo "Install root: $INSTALL_ROOT"
echo ""

fail() {
    echo "Error: $*" >&2
    if [ -d "$INSTALL_ROOT" ]; then
        echo "--- installed tree (depth 4) ---" >&2
        find "$INSTALL_ROOT" -maxdepth 4 | sort >&2
    fi
    exit 1
}

expect_path() {
    if [ ! -e "$1" ]; then
        fail "missing after install: $1"
    fi
    echo "ok: $1"
}

find_installer() {
    local matches=("$BUILD_DIR"/wisdom-chess-*."$1")
    if [ ${#matches[@]} -ne 1 ] || [ ! -e "${matches[0]}" ]; then
        fail "expected exactly one $BUILD_DIR/wisdom-chess-*.$1, found: ${matches[*]}"
    fi
    INSTALLER="${matches[0]}"
    echo "Installer: $INSTALLER ($(du -h "$INSTALLER" | cut -f1))"
}

rm -rf "$INSTALL_ROOT"

case "$(uname -s)" in
    Linux)
        find_installer run
        chmod +x "$INSTALLER"
        "$INSTALLER" "${IFW_ARGS[@]}"

        expect_path "$INSTALL_ROOT/bin/WisdomChessQml"
        expect_path "$INSTALL_ROOT/bin/qt.conf"
        expect_path "$INSTALL_ROOT/lib/libQt6Core.so.6"
        expect_path "$INSTALL_ROOT/plugins/platforms/libqxcb.so"
        expect_path "$INSTALL_ROOT/qml/QtQuick/Controls/qmldir"
        expect_path "$INSTALL_ROOT/share/icons/wisdom-chess.png"
        expect_path "${XDG_DATA_HOME:-$HOME/.local/share}/applications/wisdom-chess.desktop"

        if ldd "$INSTALL_ROOT/bin/WisdomChessQml" | grep "not found"; then
            fail "the installed executable has unresolved shared libraries"
        fi
        echo "ok: all shared libraries of bin/WisdomChessQml resolve"

        MAINTENANCE_TOOL="$INSTALL_ROOT/WisdomChessMaintenanceTool"
        ;;

    Darwin)
        find_installer dmg
        MOUNT_POINT="$INSTALL_ROOT-dmg"
        hdiutil attach "$INSTALLER" -nobrowse -mountpoint "$MOUNT_POINT"
        trap 'hdiutil detach "$MOUNT_POINT" -quiet || true' EXIT

        INSTALLER_APPS=("$MOUNT_POINT"/*.app)
        [ ${#INSTALLER_APPS[@]} -eq 1 ] || fail "expected one .app in the disk image, found: ${INSTALLER_APPS[*]}"
        INSTALLER_BINARIES=("${INSTALLER_APPS[0]}"/Contents/MacOS/*)
        "${INSTALLER_BINARIES[0]}" "${IFW_ARGS[@]}"

        APP="$INSTALL_ROOT/WisdomChessQml.app"
        expect_path "$APP/Contents/MacOS/WisdomChessQml"
        expect_path "$APP/Contents/Frameworks/QtCore.framework"
        expect_path "$APP/Contents/PlugIns/platforms/libqcocoa.dylib"
        [ -n "$(find "$APP/Contents/Resources/qml/QtQuick/Controls" -name qmldir 2>/dev/null | head -1)" ] \
            || fail "no qmldir under $APP/Contents/Resources/qml/QtQuick/Controls"
        echo "ok: QtQuick.Controls QML module deployed"
        plutil -p "$APP/Contents/Info.plist" | grep -q '"CFBundleName" => "Wisdom Chess"' \
            || fail "CFBundleName is not 'Wisdom Chess'"
        echo "ok: CFBundleName"

        MAINTENANCE_TOOL="$INSTALL_ROOT/WisdomChessMaintenanceTool.app/Contents/MacOS/WisdomChessMaintenanceTool"
        ;;

    MINGW*|MSYS*|CYGWIN*)
        find_installer exe
        "$INSTALLER" "${IFW_ARGS[@]}"

        expect_path "$INSTALL_ROOT/bin/WisdomChessQml.exe"
        expect_path "$INSTALL_ROOT/bin/qt.conf"
        expect_path "$INSTALL_ROOT/bin/Qt6Core.dll"
        expect_path "$INSTALL_ROOT/bin/vc_redist.x64.exe"
        expect_path "$INSTALL_ROOT/plugins/platforms/qwindows.dll"
        expect_path "$INSTALL_ROOT/qml/QtQuick/Controls/qmldir"

        MAINTENANCE_TOOL="$INSTALL_ROOT/WisdomChessMaintenanceTool.exe"
        ;;

    *)
        fail "unsupported platform: $(uname -s)"
        ;;
esac

echo ""
echo "Installed size: $(du -sh "$INSTALL_ROOT" | cut -f1)"

echo ""
echo "=== Uninstalling with the maintenance tool ==="
expect_path "$MAINTENANCE_TOOL"
"$MAINTENANCE_TOOL" --platform minimal --confirm-command purge
if [ -e "$INSTALL_ROOT/bin" ] || [ -e "$INSTALL_ROOT/WisdomChessQml.app" ]; then
    fail "purge left the installed files behind in $INSTALL_ROOT"
fi
echo "ok: purge removed the installation"
echo ""
echo "=== Smoke test passed ==="
