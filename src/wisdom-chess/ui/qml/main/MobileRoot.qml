import QtQuick 
import QtQuick.Controls 
import QtQuick.Layouts 
import "../popups"

Item {
    width: parent.width
    height: parent.height

    readonly property bool anyDialogOpen: dialogs.anyDialogOpen
    readonly property int animationDelay: 200 // milliseconds
    property int toolbarHeight

    property var currentFocusedItem: null

    function onFocusObjectChanged(oldFocusItem, newFocusItem) {
        boards.onFocusObjectChanged(oldFocusItem, newFocusItem)
    }

    function showNewGameDialog() {
        dialogs.showNewGameDialog()
    }

    function showAboutDialog() {
        dialogs.showAboutDialog()
    }

    function showSettingsDialog() {
        dialogs.showSettingsDialog()
    }

    Flickable {
        property int totalContentHeight: boards.height + boardStatusBar.implicitHeight + 150

        anchors.fill: parent
        contentHeight: Math.max(parent.height, totalContentHeight)
        contentWidth: parent.width
        height: parent.height
        width: parent.width

        Board {
            id: boards
            anchors.centerIn: parent
        }

        StatusBar {
            id: boardStatusBar
            anchors.top: boards.bottom
            anchors.topMargin: 25
            anchors.left: parent.left
            anchors.right: parent.right
        }
    }

    Dialogs {
        id: dialogs
    }

}

