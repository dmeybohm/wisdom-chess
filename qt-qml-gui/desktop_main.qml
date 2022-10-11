import QtQuick 
import wisdom.chess 1.0
import QtQuick.Controls 
import QtQuick.Layouts 

import "Helper.js" as Helper

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

            Item {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft;
            }

            ImageToolButton {
                Layout.alignment: Qt.AlignRight;
                Layout.fillHeight: true
                Layout.rightMargin: 2
                implicitWidth: 25
                implicitHeight: 25
                imageSource: "images/bx-icon-menu.svg"
                onClicked: settingsMenu.open()
            }
        }
    }

    DesktopRoot {
        id: root

        SettingsMenu {
            id: settingsMenu
            x: root.width - settingsMenu.width
            onShowAboutDialog: root.showAboutDialog()
            onShowNewGameDialog: root.showNewGameDialog()
        }

    }

    BoardDimensions {
        id: boardDimensions
    }
}
