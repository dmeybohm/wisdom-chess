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
        id: threefoldRepetitionDialog
        visible: !userAnswered && _myGameModel.thirdRepetitionDrawProposed
        anchors.centerIn: parent
        width: Math.min(400, Screen.width - 50)
        height: Math.min(200, Screen.height - 10)
        padding: 40
        text: "The same position has been repeated three times."

        // hide the dialog and break the property binding:
        onAccepted: {
            _myGameModel.humanWantsThreefoldRepetitionDraw(true)
            userAnswered = true
        }
        onRejected: {
            _myGameModel.humanWantsThreefoldRepetitionDraw(false)
            userAnswered = true
        }
    }

    DrawProposalDialog {
        id: fiftyMovesNoProgressDrawDialog
        visible: !userAnswered && _myGameModel.fiftyMovesWithoutProgressDrawProposed
        anchors.centerIn: parent
        width: Math.min(400, Screen.width - 50)
        height: Math.min(200, Screen.height - 10)
        padding: 40
        text: "There have been fifty moves without a capture or pawn move."

        // hide the dialog and break the property binding:
        onAccepted: {
            _myGameModel.humanWantsFiftyMovesWithoutProgressDraw(true)
            userAnswered = true
        }
        onRejected: {
            _myGameModel.humanWantsFiftyMovesWithoutProgressDraw(false)
            userAnswered = true
        }
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

