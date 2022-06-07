import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    width: parent.width
    height: parent.height

    readonly property int animationDelay: 200 // milliseconds
    property int toolbarHeight

    property var currentFocusedItem: null

    function onFocusObjectChanged(oldFocusItem, newFocusItem) {
        boards.onFocusObjectChanged(oldFocusItem, newFocusItem)
    }

    function showNewGameDialog() {
        dialogs.showNewGameDialog()
    }

    Flickable {
        id: colLayout

        property int totalContentHeight: boards.height + boardStatusBar.implicitHeight + 50

        anchors.fill: parent
        contentHeight: Math.max(Screen.height - toolbar.height, totalContentHeight)
        contentWidth: Screen.width
        height: Screen.height - toolbar.height
        width: Screen.width

        Component.onCompleted: {
            console.log('toolbar.height: ' +toolbar.height)
            console.log ('colLayout.height: '+colLayout.height)
            console.log('Screen.height: '+Screen.height)
            console.log('Screen.height adjusted: '+ (Screen.height - toolbar.height))
            console.log('contentHeight: '+totalContentHeight)
        }

        Rectangle {
            color: "red"
            anchors.fill: parent
        }

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

