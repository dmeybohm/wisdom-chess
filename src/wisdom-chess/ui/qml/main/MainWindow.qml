import QtQuick
import QtQuick.Controls

// What the desktop, mobile and wasm mains share. Each lays out its own
// header, root and menu and hands the last two over.
ApplicationWindow {
    id: mainWindow

    required property GameRoot gameRoot
    required property GameMenu menu

    visible: true
    title: qsTr("Wisdom Chess")
    color: "silver"

    // The engine holds its move while a menu or a dialog is open.
    readonly property bool anyPopupOpen: menu.visible || gameRoot.anyDialogOpen

    onAnyPopupOpenChanged: {
        if (anyPopupOpen)
            _myGameModel.pause()
        else
            _myGameModel.unpause()
    }

    onClosing: _myGameModel.applicationExiting()
}
