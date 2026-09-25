import QtQuick 
import QtQuick.Controls 

Dialog {
    id: topDialog
    modal: true
    standardButtons: Dialog.Yes | Dialog.No
    title: "New Game"
    width: Math.min(400, Screen.width - 50)
    padding: 40

    onAccepted: {
        GameModel.restart()
        visible = false
    }
    onRejected: {
        visible = false
    }

    Text {
        id: firstLine
        text: "Start a new game?"
        font.pointSize: 16
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
    }
}
