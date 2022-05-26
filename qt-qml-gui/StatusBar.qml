import QtQuick 2.15
import QtQuick.Layouts 1.15
import wisdom.chess 1.0

ColumnLayout {
    id: statusBar
    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.topMargin: 35
    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
    spacing: 5
    property int fontSize: 16

    Text {
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
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
        verticalAlignment: Text.AlignVCenter
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
