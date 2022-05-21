import QtQuick 2.15
import wisdom.chess 1.0

Item {
    height: 80
    anchors.left: parent.left
    anchors.right: parent.right

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        spacing: 5

        Text {
            width: parent.width
            height: parent.height / 2
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 14
            color: "#ff333333"
            text: _myGameModel.gameOverStatus !== ""
              ? _myGameModel.gameOverStatus :
               _myGameModel.currentTurn === ChessColor.White ?
             "White to move" :
             "Black to move"
        }

        Text {
            id: moveStatus
            width: parent.width
            font.pointSize: 14
            color: "#ff333333"
            text: _myGameModel.moveStatus
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

}
