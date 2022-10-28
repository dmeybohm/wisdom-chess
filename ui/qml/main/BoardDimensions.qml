import QtQml
import QtQuick
import WisdomChess

QtObject {
    //
    // Constant settings.
    //
    readonly property int boardWidth: squareSize * 8
    readonly property int boardHeight: boardWidth
    readonly property int totalSquares: 8 * 8

    // The square size, which gets updated based on screen size.
    property int squareSize: calculateMaxSquareSize()

    function calculateMaxSquareSize() {
        const maxWidth = (Screen.width - 20) / 8
        const maxHeight = (Screen.height - 20) / 8
        return Math.min(maxWidth, maxHeight, 64)
    }
}
