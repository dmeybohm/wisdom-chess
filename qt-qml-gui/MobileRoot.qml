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
        id: colLayout

        property int totalContentHeight: boards.height + boardStatusBar.implicitHeight + 25

        anchors.fill: parent
        contentHeight: Math.max(Screen.height - toolbar.height - 40, totalContentHeight)
        contentWidth: Screen.width
        height: Math.min(Screen.height, contentHeight)
        width: Screen.width

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

