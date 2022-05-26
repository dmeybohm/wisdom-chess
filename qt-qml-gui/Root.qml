import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    width: parent.width
    height: parent.height

    property int squareSize: topWindow.calculateMaxSquareSize()
    readonly property int boardWidth: squareSize * 8
    readonly property int boardHeight: boardWidth
    readonly property int totalSquares: 8 * 8
    readonly property int animationDelay: 200 // millisecondss

    property var currentFocusedItem: null

    function onFocusObjectChanged(oldFocusItem, newFocusItem) {
        boards.onFocusObjectChanged(oldFocusItem, newFocusItem)
    }

    function showNewGameDialog() {
        newGameDialog.visible = true
    }

    Flickable {
        id: scrollView
        y: 20
        x: (parent.width - boardWidth) / 2
        width: Math.max(boardWidth, Screen.width)
        height: Math.min(Screen.height, boardHeight + 200)
        contentWidth: boardWidth
        contentHeight: boardHeight + 200
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Board {
                id: boards
            }

            StatusBar {
                id: boardStatusBar
            }
        }
    }

    DrawProposalDialog {
        id: acceptDrawDialog
        anchors.centerIn: parent
        width: Math.min(400, Screen.width - 50)
        height: Math.min(200, Screen.height - 10)
        padding: 40
        text: "Your opponent has repeated the same move three times."
    }

    NewGameDialog {
        id: newGameDialog
        visible: false
        anchors.centerIn: parent
        width: Math.min(400, Screen.width - 50)
        height: Math.min(150, Screen.height - 10)
        padding: 40
    }

}
