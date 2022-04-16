import QtQuick 2.15
import wisdom.chess 1.0
import QtQuick.Controls 2.15

Window {
    id: root

    readonly property int squareSize: 64
    readonly property int boardWidth: squareSize * 8
    readonly property int boardHeight: boardWidth
    readonly property int totalSquares: 8 * 8
    readonly property int animationDelay: 200 // millisecondss

    width: boardWidth + 48
    height: boardHeight + 48 + 48
    visible: true
    title: qsTr("Wisdom Chess")
    color: "silver"

    property var currentFocusedItem: null
    onFocusObjectChanged: {
        console.log(activeFocusItem)
        console.log(currentFocusedItem)
        boards.onFocusObjectChanged(root.currentFocusedItem, activeFocusItem)
        root.currentFocusedItem = activeFocusItem
    }

    onClosing: {
        _myGameModel.applicationExiting();
    }

    Column {
        anchors.top: parent.top
        anchors.topMargin: 25
        anchors.horizontalCenter: parent.horizontalCenter

        Board {
            id: boards
        }

        StatusBar {
        }
    }

    Dialog {
        id: acceptDrawDialog
        anchors.centerIn: parent
        width: 400
        height: 200
        padding: 40
        modal: true
        visible: _myGameModel.drawProposedToHuman
        standardButtons: Dialog.Yes | Dialog.No
        title: "Draw Offer"

        onAccepted: {
            _myGameModel.drawProposalResponse(true)
        }
        onRejected: {
            _myGameModel.drawProposalResponse(false)
        }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right

            spacing: 15
            Text {
                anchors.left: parent.left;
                anchors.right: parent.right;
                text: "Your opponent has proposed a draw."
                horizontalAlignment: Text.AlignHCenter

            }
            Text {
                anchors.left: parent.left;
                anchors.right: parent.right;
                text: "Would you like to accept?"
                horizontalAlignment: Text.AlignHCenter
            }


        }

    }
}

