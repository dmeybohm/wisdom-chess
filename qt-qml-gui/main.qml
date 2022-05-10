import QtQuick 2.15
import wisdom.chess 1.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: root

    width: Screen.width
    height: Screen.height

    function calculateMaxSquareSize() {
        const maxWidth = (Screen.width - 20) / 8
        const maxHeight = (Screen.height - 20) / 8
        console.log('maxWidth: '+maxWidth)
        console.log('maxHeight:'+maxHeight)
        return Math.min(maxWidth, maxHeight, 64)
    }

    property int squareSize: calculateMaxSquareSize()
    readonly property int boardWidth: squareSize * 8
    readonly property int boardHeight: boardWidth
    readonly property int totalSquares: 8 * 8
    readonly property int animationDelay: 200 // millisecondss

    //width: boardWidth + 48
    //height: boardHeight + 48 + 48
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

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            Label {
                text: "Wisdom Chess"
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.leftMargin: 17
            }
            ToolButton {
                text: qsTr("⋮")
                onClicked: menu.open()
            }
        }
    }

    onClosing: {
        _myGameModel.applicationExiting();
    }

    Screen.onPrimaryOrientationChanged:{
        root.squareSize = calculateMaxSquareSize()
        console.log("new square size: "+root.squareSize)
    }

    Flickable {
        id: scrollView
        x: (Screen.width - boardWidth) / 2
        y: 20
        /*
        anchors.top: parent.top
        anchors.topMargin: 25
        */
        width: Math.max(boardWidth, Screen.width)
        height: Math.min(Screen.height, boardHeight + 200)
        contentWidth: boardWidth
        contentHeight: boardHeight + 200
        boundsBehavior: Flickable.StopAtBounds

        /*
        onWidthChanged: {
            console.log('screen size updated')
            implicitWidth = Math.min(Screen.width, boardWidth)
            implicitHeight = Math.min(Screen.height, boardHeight + 200)
            console.log('implicitWidth: '+implicitWidth)
            console.log('implicitHeight: '+implicitHeight)
        }

        */

        Column {
            spacing: 15

            Board {
                id: boards
            }

            StatusBar {
                id: boardStatusBar
            }
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

