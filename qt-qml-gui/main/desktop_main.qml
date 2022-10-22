import QtQuick 
import WisdomChess 1.0
import QtQuick.Controls 
import QtQuick.Layouts 

import "../Helper.js" as Helper

ApplicationWindow {
    id: topWindow

    readonly property int boardWidth: boardDimensions.boardWidth
    readonly property int boardHeight: boardDimensions.boardHeight
    readonly property int squareSize: boardDimensions.squareSize

    width: boardWidth + 48
    height: boardHeight + 48 + 145

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

    header: ToolBar {
        id: toolbar
        height: 35

        RowLayout {
            anchors.fill: parent

            Label {
                visible: Helper.isWebAssembly()
                text: "Wisdom Chess"
                elide: Label.ElideRight
                Layout.alignment: Qt.AlignLeft;
                Layout.leftMargin: 17
                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter
            }

            ImageToolButton {
                Layout.alignment: Qt.AlignLeft;
                Layout.fillHeight: true
                Layout.rightMargin: 2
                implicitWidth: 30
                implicitHeight: 30
                imageSource: "../images/bx-icon-menu.svg"
                onClicked: gameMenu.open()
            }

            ImageToolButton {
                Layout.alignment: Qt.AlignRight;
                Layout.fillHeight: true
                Layout.rightMargin: 2
                implicitWidth: 30
                implicitHeight: 30
                imageSource: "../images/bxs-cog.svg"
                onClicked: settingsMenu.open()
            }

        }
    }

    DesktopRoot {
        id: root

        GameMenu {
            id: gameMenu
            x: 0
            onShowAboutDialog: root.showAboutDialog()
            onShowNewGameDialog: root.showNewGameDialog()
            onQuit: root.showConfirmQuitDialog()
        }

        SettingsMenu {
            id: settingsMenu
            x: root.width - settingsMenu.width
        }

    }

    BoardDimensions {
        id: boardDimensions
    }
}
