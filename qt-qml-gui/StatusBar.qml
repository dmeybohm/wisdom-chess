import QtQuick 2.0
import wisdom.chess 1.0

Item {
    width: root.squareSize
    height: root.squareSize

    Column {
        Text {
            width: implicitWidth
            height: implicitHeight
            text: _myGameModel.currentTurn === Color.White ? "White to move" :
                         "Black to move"
        }

        Text {
            width: implicitWidth
            height: implicitHeight
            text: _myGameModel.gameStatus
        }
    }
}
