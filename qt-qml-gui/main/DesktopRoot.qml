import QtQuick 
import QtQuick.Controls 
import QtQuick.Layouts 
import WisdomChess

Item {
    width: parent.width
    height: parent.height

    readonly property int animationDelay: 200 // milliseconds

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

    function showConfirmQuitDialog() {
        dialogs.showConfirmQuitDialog()
    }

    ColumnLayout {
        id: colLayout
        anchors.top: parent.top
        anchors.topMargin: 25
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 25

        Board {
            id: boards

            Layout.alignment: Qt.AlignHCenter
        }

        StatusBar {
            id: boardStatusBar
            Layout.fillWidth: true
            Layout.minimumHeight: 50
        }
    }

    Dialogs {
        id: dialogs
    }
}
