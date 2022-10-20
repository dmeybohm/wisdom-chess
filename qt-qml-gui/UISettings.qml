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

    //
    // Changeable settings:
    //
    property int whitePlayer: Player.Human
    property int blackPlayer: Player.Computer
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
    onWhitePlayerChanged: {
        _myGameModel.setWhitePlayer(whitePlayer)
    }

    onBlackPlayerChanged: {
        _myGameModel.setBlackPlayer(blackPlayer)
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
        _myGameModel.setWhiteIsComputer(whitePlayer === Player.Computer)
        _myGameModel.setBlackIsComputer(blackPlayer === Player.Computer)
        _myGameModel.setMaxDepth(maxDepth)
        _myGameModel.setMaxSearchTime(maxSearchTime)
    }
}
