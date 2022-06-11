import QtQuick 2.0

Item {
    anchors.fill: parent

    function showNewGameDialog() {
        newGameDialog.visible = true
    }

    DrawProposalDialog {
        id: threefoldRepetitionDialog
        visible: !userAnswered && _myGameModel.thirdRepetitionDrawProposed
        anchors.centerIn: parent
        width: Math.min(400, Screen.width - 50)
        height: Math.min(250, Screen.height - 10)
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
        height: Math.min(250, Screen.height - 10)
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
