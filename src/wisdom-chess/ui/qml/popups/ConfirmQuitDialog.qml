import QtQuick
import QtQuick.Controls

Dialog {
    id: topDialog
    modal: true
    standardButtons: Dialog.Yes | Dialog.No
    title: "Quit Wisdom Chess"
    width: Math.min(500, Screen.width - 50)
    padding: 40

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
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
    }
}
