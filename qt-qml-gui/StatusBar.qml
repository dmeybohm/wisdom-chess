import QtQuick
import QtQuick.Layouts 
import wisdom.chess 1.0

ColumnLayout {
    id: statusBar
    spacing: 5
    implicitHeight: toMove.implicitHeight + moveStatus.implicitHeight + 80
    property int fontSize: 16

    Text {
        id: toMove
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: statusBar.fontSize
        Layout.fillWidth: true
        color: "#ff333333"
        text: _myGameModel.gameOverStatus !== ""
          ? _myGameModel.gameOverStatus :
           _myGameModel.currentTurn === ChessColor.White ?
         "<b>White</b> to move" :
         "<b>Black</b> to move"
    }

    Text {
        id: moveStatus
        horizontalAlignment: Text.AlignHCenter
        Layout.fillWidth: true
        font.pointSize: statusBar.fontSize
        color: "#ff333333"
        text: _myGameModel.moveStatus +
          (
              Boolean(_myGameModel.moveStatus) && Boolean(_myGameModel.inCheck) ?
              " - " : ""
          ) +
          (
              (_myGameModel.inCheck && _myGameModel.gameOverStatus === "") ? "Check!" : ""
          )
    }
}
