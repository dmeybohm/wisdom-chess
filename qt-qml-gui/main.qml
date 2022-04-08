import QtQuick
import wisdom.chess 1.0

Window {
    id: root

    readonly property int squareSize: 90
    readonly property int boardWidth: squareSize * 8
    readonly property int boardHeight: boardWidth
    readonly property int totalSquares: 8 * 8
    readonly property int animationDelay: 200 // millisecondss

    width: boardWidth + 50
    height: boardHeight + 50 + 50
    visible: true
    title: qsTr("Wisdom Chess")
    color: "silver"

    property var currentFocusedItem: null
    onFocusObjectChanged: {
        console.log(activeFocusItem)
        console.log(currentFocusedItem)
        boards.onFocusObjectChanged(root.currentFocusedItem, activeFocusItem)
        root.currentFocusedItem = activeFocusItem
    }

    onClosing: {
        _myGameModel.applicationExiting();
    }

    Column {
        x: 25
        y: 25
        spacing: 25

        Board {
            id: boards
        }

        StatusBar {

        }
    }
}

