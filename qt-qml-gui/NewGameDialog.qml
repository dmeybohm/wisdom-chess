import QtQuick 2.15
import QtQuick.Controls 2.15

Dialog {
    modal: true
    standardButtons: Dialog.Yes | Dialog.No
    title: "New Game"

    onAccepted: {
        _myGameModel.restart()
        visible = false
    }
    onRejected: {
        visible = false
    }

    Text {
        id: firstLine
        text: "Start a new game?"
        anchors.centerIn: parent
        font.pointSize: 16
        horizontalAlignment: Text.AlignHCenter
        width: parent.width
        wrapMode: Text.WordWrap
    }
}
