import QtQuick
import WisdomChess

import "../Helper.js" as Helper

Item {
    anchors.fill: parent

    function showNewGameDialog() {
        newGameDialog.visible = true
    }

    function showAboutDialog() {
        aboutDialog.visible = true
    }

    function showConfirmQuitDialog() {
        confirmQuitDialog.visible = true
    }

    function showSettingsDialog() {
        settingsDialog.visible = true
    }

    SettingsDialog {
        id: settingsDialog
        visible: false
        anchors.centerIn: parent
    }

    DrawProposalDialog {
        id: threefoldRepetitionDialog
        visible: _myGameModel.thirdRepetitionDrawStatus == DrawByRepetitionStatus.Proposed
        anchors.centerIn: parent
        width: Math.min(400, Screen.width - 50)
        height: Math.min(250, Screen.height - 10)
        padding: 40
        text: "The same position has been repeated three times."

        // hide the dialog and break the property binding:
        onAccepted: {
            _myGameModel.thirdRepetitionDrawStatus = DrawByRepetitionStatus.Accepted
        }
        onRejected: {
            _myGameModel.thirdRepetitionDrawStatus = DrawByRepetitionStatus.Declined
        }
    }

    DrawProposalDialog {
        id: fiftyMovesNoProgressDrawDialog
        visible: _myGameModel.fiftyMovesDrawStatus == DrawByRepetitionStatus.Proposed
        anchors.centerIn: parent
        width: Math.min(400, Screen.width - 50)
        height: Math.min(250, Screen.height - 10)
        padding: 40
        text: "There have been fifty moves without a capture or pawn move."

        // hide the dialog and break the property binding:
        onAccepted: {
            _myGameModel.fiftyMovesDrawStatus = DrawByRepetitionStatus.Accepted
        }
        onRejected: {
            _myGameModel.fiftyMovesDrawStatus = DrawByRepetitionStatus.Declined
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

    AboutDialog {
        id: aboutDialog
        visible: false
        anchors.centerIn: parent
    }

    ConfirmQuitDialog {
        id: confirmQuitDialog
        visible: false
        width: Math.min(500, Screen.width - 50)
        height: Math.min(150, Screen.height - 10)
        anchors.centerIn: parent
    }
}
