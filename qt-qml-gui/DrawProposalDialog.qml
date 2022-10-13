import QtQuick 
import wisdom.chess 1.0
import QtQuick.Controls 

Dialog {
    property alias text: firstLine.text

    modal: true
    standardButtons: Dialog.Yes | Dialog.No
    title: "Draw Offer"

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
            width: parent.width
            wrapMode: Text.WordWrap
        }
    }
}
