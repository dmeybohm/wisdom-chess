pragma Singleton

import QtQml

// Which build this is, for the few places the layout differs.
QtObject {
    // Qt.platform is loosely typed in Qt itself.
    // qmllint disable compiler
    readonly property bool isMobile: Qt.platform.os === "android"
    readonly property bool isWebAssembly: Qt.platform.os === "wasm"
    readonly property bool isDesktop: !isMobile && !isWebAssembly
    readonly property bool isMacOS: Qt.platform.os === "osx"
    // qmllint enable compiler
}
