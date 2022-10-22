import QtQuick
import QtQuick.Controls
import "../Helper.js" as Helper

Dialog {
    id: topDialog
    modal: true
    standardButtons: Dialog.Yes | Dialog.No
    title: "Quit Wisdom Chess"

    onAccepted: {
        Qt.quit()
    }
    onRejected: {
        visible = false
    }

    Text {
        id: firstLine
        text: "Are you sure you want to end the game?"
        font.pointSize: 16
        anchors.fill: parent

        verticalAlignment: Helper.isMobile() ? Text.AlignTop : Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
    }
}
