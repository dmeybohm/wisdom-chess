import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

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

//    Dialogs {
//        id: dialogs
//    }
}
