import QtQuick

Item {
    anchors.fill: parent

    readonly property bool anyDialogOpen: settingsDialog.visible
        || newGameDialog.visible
        || aboutDialog.visible
        || confirmQuitDialog.visible

    function showNewGameDialog(): void {
        newGameDialog.visible = true
    }

    function showAboutDialog(): void {
        aboutDialog.visible = true
    }

    function showConfirmQuitDialog(): void {
        confirmQuitDialog.visible = true
    }

    function showSettingsDialog(): void {
        settingsDialog.visible = true
    }

    SettingsDialog {
        id: settingsDialog
        visible: false
        anchors.centerIn: parent
    }

    DrawProposalDialog {
        id: threefoldRepetitionDialog
        visible: GameModel.thirdRepetitionDrawStatus == DrawByRepetitionStatus.Proposed
        anchors.centerIn: parent
        text: "The same position has been repeated three times."

        // hide the dialog and break the property binding:
        onAccepted: {
            GameModel.thirdRepetitionDrawStatus = DrawByRepetitionStatus.Accepted
        }
        onRejected: {
            GameModel.thirdRepetitionDrawStatus = DrawByRepetitionStatus.Declined
        }
    }

    DrawProposalDialog {
        id: fiftyMovesNoProgressDrawDialog
        visible: GameModel.fiftyMovesDrawStatus == DrawByRepetitionStatus.Proposed
        anchors.centerIn: parent
        text: "There have been fifty moves without a capture or pawn move."

        // hide the dialog and break the property binding:
        onAccepted: {
            GameModel.fiftyMovesDrawStatus = DrawByRepetitionStatus.Accepted
        }
        onRejected: {
            GameModel.fiftyMovesDrawStatus = DrawByRepetitionStatus.Declined
        }
    }

    NewGameDialog {
        id: newGameDialog
        visible: false
        anchors.centerIn: parent
    }

    AboutDialog {
        id: aboutDialog
        visible: false
        anchors.centerIn: parent
    }

    ConfirmQuitDialog {
        id: confirmQuitDialog
        visible: false
        anchors.centerIn: parent
    }
}
