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
            /*
            Button {
                text: "open dialog"
                width: 200
                height: 25
                onClicked: {
                   acceptDrawDialog.open()
                }
            }
            */
        }

        Dialog {
            id: acceptDrawDialog
            anchors.centerIn: parent
            width: 400
            height: 200
            padding: 40
            modal: true
            standardButtons: Dialog.Yes | Dialog.No
            title: "Draw Offer"

            Column {
                spacing: 15
                Text {
                    text: "Your opponent has proposed a draw."
                }
                Text {
                    text: "Would you like to accept?"
                }
            }

        }
    }
}

