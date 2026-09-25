import QtQuick

// What the desktop and mobile roots share: the dialogs, and the focus
// tracking that turns a click on one square and then another into a move.
Item {
    id: gameRoot

    // The board the derived root lays out.
    required property Board board

    readonly property alias dialogs: dialogs
    readonly property bool anyDialogOpen: dialogs.anyDialogOpen

    width: parent.width
    height: parent.height

    // The item that last had active focus in the window. A change is a
    // square losing focus and another gaining it, or a dialog taking it.
    // This is a signal handler rather than a bound property because the
    // board clears the new item's focus, which a binding would loop on.
    property Item previousFocusedItem: null

    Connections {
        target: gameRoot.Window.window

        function onActiveFocusItemChanged() {
            const focusedItem = gameRoot.Window.window.activeFocusItem
            gameRoot.board.focusMoved(gameRoot.previousFocusedItem, focusedItem)
            gameRoot.previousFocusedItem = focusedItem
        }
    }

    Dialogs {
        id: dialogs
    }
}
