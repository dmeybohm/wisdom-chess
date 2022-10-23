import QtQuick 
import WisdomChess
import QtQuick.Controls 
import QtQuick.Layouts 
import "../popups"
import "../Helper.js" as Helper

ApplicationWindow {
    id: topWindow

    readonly property int boardWidth: boardDimensions.boardWidth
    readonly property int boardHeight: boardDimensions.boardHeight
    readonly property int squareSize: boardDimensions.squareSize

    readonly property bool isWebAssembly: Helper.isWebAssembly()
    readonly property bool isMobile: Helper.isMobile()

    width: Screen.width
    height: Screen.height

    visible: true
    title: qsTr("Wisdom Chess")
    color: "silver"

    property var currentFocusedItem: null
    onFocusObjectChanged: {
        root.onFocusObjectChanged(root.currentFocusedItem, activeFocusItem)
        root.currentFocusedItem = activeFocusItem
    }

    onClosing: {
        _myGameModel.applicationExiting();
    }

    Screen.onPrimaryOrientationChanged: {
        boardDimensions.squareSize = boardDimensions.calculateMaxSquareSize()
        console.log("new square size: "+uiSettings.squareSize)
    }

    header: ToolBar {
        id: toolbar

        RowLayout {
            anchors.fill: parent

            Image {
                source: "../images/Chess_rlt45.svg"
                Layout.maximumWidth: 32
                Layout.maximumHeight: 32

                Layout.alignment: Qt.AlignLeft
                Layout.leftMargin: 12
            }

            Label {
                text: "Wisdom Chess"
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.leftMargin: 2
            }

            ImageToolButton {
                Layout.alignment: Qt.AlignRight;
                Layout.fillHeight: true
                Layout.rightMargin: 10
                implicitWidth: 25
                implicitHeight: 25
                imageSource: "../images/bx-icon-menu-white.png"
                onClicked: gameMenu.visible ? gameMenu.close() : gameMenu.open()
            }
        }
    }

    MobileRoot {
        id: root
        toolbarHeight: toolbar.height
        anchors.fill: parent

        Flickable {

            visible: gameMenu.visible
            width: gameMenu.width
            height: Math.min(Screen.height, gameMenu.height)

            GameMenu {
                id: gameMenu
                x: root.width - gameMenu.width
                onShowNewGameDialog: root.showNewGameDialog();
                onShowAboutDialog: root.showAboutDialog();
                onShowSettingsDialog: root.showSettingsDialog();
            }
        }
    }

    BoardDimensions {
        id: boardDimensions
    }
}
