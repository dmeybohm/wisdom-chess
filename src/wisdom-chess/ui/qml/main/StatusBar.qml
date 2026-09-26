import QtQuick
import QtQuick.Layouts

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
        text: GameModel.gameOverStatus !== ""
          ? GameModel.gameOverStatus :
           GameModel.currentTurn === Color.White ?
         "<b>White</b> to move" :
         "<b>Black</b> to move"
    }

    Text {
        id: moveStatus
        horizontalAlignment: Text.AlignHCenter
        Layout.fillWidth: true
        font.pointSize: statusBar.fontSize
        color: "#ff333333"
        text: GameModel.moveStatus +
          (
              Boolean(GameModel.moveStatus) && Boolean(GameModel.inCheck) ?
              " - " : ""
          ) +
          (
              (GameModel.inCheck && GameModel.gameOverStatus === "") ? "Check!" : ""
          )
    }
}
