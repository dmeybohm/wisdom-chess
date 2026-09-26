import QtQml
import QtQuick

QtObject {
    //
    // Constant settings.
    //
    readonly property int boardWidth: squareSize * 8
    readonly property int boardHeight: boardWidth

    // The square size, which gets updated based on screen size.
    property int squareSize: calculateMaxSquareSize()

    function calculateMaxSquareSize() {
        const maxWidth = (Screen.width - 20) / 8
        const maxHeight = (Screen.height - 20) / 8
        return Math.min(maxWidth, maxHeight, 64)
    }
}
