import QtQuick

// What the desktop and mobile roots share: the dialogs, the menu's way
// into them, and the pause while either is open.
Item {
    id: gameRoot

    // The menu the derived main lays out.
    required property GameMenu menu

    readonly property alias dialogs: dialogs

    width: parent.width
    height: parent.height

    // The engine holds its move while a menu or a dialog is open.
    readonly property bool anyPopupOpen: menu.visible || dialogs.anyDialogOpen

    onAnyPopupOpenChanged: {
        if (anyPopupOpen)
            GameModel.pause()
        else
            GameModel.unpause()
    }

    Connections {
        target: gameRoot.menu

        function onShowNewGameDialog(): void {
            dialogs.showNewGameDialog()
        }

        function onShowAboutDialog(): void {
            dialogs.showAboutDialog()
        }

        function onShowSettingsDialog(): void {
            dialogs.showSettingsDialog()
        }

        function onQuit(): void {
            dialogs.showConfirmQuitDialog()
        }
    }

    Dialogs {
        id: dialogs
    }
}
