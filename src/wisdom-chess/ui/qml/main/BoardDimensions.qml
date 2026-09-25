pragma Singleton

import QtQuick

QtObject {
    readonly property int boardWidth: squareSize * 8
    readonly property int boardHeight: boardWidth

    // The square size, which the mobile main recomputes when the screen
    // turns.
    property int squareSize: calculateMaxSquareSize()

    function calculateMaxSquareSize() {
        const maxWidth = (Screen.width - 20) / 8
        const maxHeight = (Screen.height - 20) / 8
        return Math.min(maxWidth, maxHeight, 64)
    }
}
