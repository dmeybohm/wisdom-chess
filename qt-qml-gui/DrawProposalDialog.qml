import QtQuick 2.15
import wisdom.chess 1.0
import QtQuick.Controls 2.15

Dialog {
    modal: true
    visible: _myGameModel.drawProposedToHuman
    standardButtons: Dialog.Yes | Dialog.No
    title: "Draw Offer"

    property alias text: firstLine.text

    onAccepted: {
        _myGameModel.drawProposalResponse(true)
    }
    onRejected: {
        _myGameModel.drawProposalResponse(false)
    }

    Column {
        spacing: 15
        anchors.left: parent.left
        anchors.right: parent.right

        Text {
            id: firstLine
            horizontalAlignment: Text.AlignHCenter
            width: parent.width
            wrapMode: Text.WordWrap
        }
        Text {
            text: "Would you like to declare a draw?"
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }
}
