import QtQuick 
import QtQuick.Controls 
import "../Helper.js" as Helper

Dialog {
    id: topDialog
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
        font.pointSize: 16
        anchors.fill: parent

        verticalAlignment: Helper.isMobile() ? Text.AlignTop : Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
    }
}
