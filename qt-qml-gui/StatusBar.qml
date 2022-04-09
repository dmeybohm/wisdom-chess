import QtQuick 2.15
import wisdom.chess 1.0

Column {
    height: 68
    anchors.left: parent.left
    anchors.right: parent.right

    Text {
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: 14
        color: "#ff333333"
        text: _myGameModel.gameStatus !== "" ? _myGameModel.gameStatus : (
                                                   _myGameModel.currentTurn === Color.White ? "White to move" :
             "Black to move")
    }

}
