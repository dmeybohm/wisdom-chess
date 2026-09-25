import QtQuick

GameRoot {

    Flickable {
        property int totalContentHeight: boards.height + boardStatusBar.implicitHeight + 150

        anchors.fill: parent
        contentHeight: Math.max(parent.height, totalContentHeight)
        contentWidth: parent.width

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
}
