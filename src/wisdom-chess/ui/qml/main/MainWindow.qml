import QtQuick
import QtQuick.Controls

// What the desktop, mobile and wasm mains share. Each lays out its own
// header, root and menu.
ApplicationWindow {
    visible: true
    title: qsTr("Wisdom Chess")
    color: "silver"

    onClosing: GameModel.applicationExiting()
}
