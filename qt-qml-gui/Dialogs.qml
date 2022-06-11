import QtQuick 2.0

Item {
    anchors.fill: parent

    function showNewGameDialog() {
        newGameDialog.visible = true
    }

    DrawProposalDialog {
        id: threefoldRepetitionDialog
        visible: !_myGameModel.thirdRepetitionDrawAnswered &&
                 _myGameModel.thirdRepetitionDrawProposed
        anchors.centerIn: parent
        width: Math.min(400, Screen.width - 50)
        height: Math.min(200, Screen.height - 10)
        padding: 40
        text: "The same position has been repeated three times."

        // hide the dialog and break the property binding:
        onAccepted: {
            _myGameModel.humanWantsThreefoldRepetitionDraw(true)
        }
        onRejected: {
            _myGameModel.humanWantsThreefoldRepetitionDraw(false)
        }
    }

    DrawProposalDialog {
        id: fiftyMovesNoProgressDrawDialog
        visible: !_myGameModel.fiftyMovesWithoutProgressDrawAnswered &&
                 _myGameModel.fiftyMovesWithoutProgressDrawProposed
        anchors.centerIn: parent
        width: Math.min(400, Screen.width - 50)
        height: Math.min(200, Screen.height - 10)
        padding: 40
        text: "There have been fifty moves without a capture or pawn move."

        // hide the dialog and break the property binding:
        onAccepted: {
            _myGameModel.humanWantsFiftyMovesWithoutProgressDraw(true)
        }
        onRejected: {
            _myGameModel.humanWantsFiftyMovesWithoutProgressDraw(false)
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
