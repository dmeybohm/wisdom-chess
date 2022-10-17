import QtQml
import QtQuick

QtObject {
    //
    // Constant settings.
    //
    readonly property int boardWidth: squareSize * 8
    readonly property int boardHeight: boardWidth
    readonly property int totalSquares: 8 * 8

    // The square size, which gets updated based on screen size.
    property int squareSize: calculateMaxSquareSize()

    //
    // Changeable settings:
    //
    property bool whiteIsComputer: false
    property bool blackIsComputer: true
    property int maxDepth: 3
    property int maxSearchTime: 3
    property bool flipped: false

    function calculateMaxSquareSize() {
        const maxWidth = (Screen.width - 20) / 8
        const maxHeight = (Screen.height - 20) / 8
        return Math.min(maxWidth, maxHeight, 64)
    }

    //
    // Update the settings on the game model when they change:
    //
    onWhiteIsComputerChanged: {
        _myGameModel.setWhiteIsComputer(whiteIsComputer)
    }

    onBlackIsComputerChanged: {
        _myGameModel.setBlackIsComputer(blackIsComputer)
    }

    onMaxDepthChanged: {
        _myGameModel.setMaxDepth(maxDepth)
    }

    onMaxSearchTimeChanged: {
        _myGameModel.setMaxSearchTime(maxSearchTime)
    }

    //
    // Initialize the settings on the game model on startup:
    //
    Component.onCompleted: {
        _myGameModel.setWhiteIsComputer(whiteIsComputer)
        _myGameModel.setBlackIsComputer(blackIsComputer)
        _myGameModel.setMaxDepth(maxDepth)
        _myGameModel.setMaxSearchTime(maxSearchTime)
    }
}
