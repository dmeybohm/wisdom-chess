import QtQuick 2.15
import wisdom.chess 1.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "Helper.js" as Helper

ApplicationWindow {
    id: topWindow

    width: root.boardWidth + 48
    height: root.boardHeight + 48 + 145

    visible: true
    title: qsTr("Wisdom Chess")
    color: "silver"

    property var currentFocusedItem: null

    onFocusObjectChanged: {
        root.onFocusObjectChanged(root.currentFocusedItem, activeFocusItem)
        root.currentFocusedItem = activeFocusItem
    }

    function calculateMaxSquareSize() {
        const maxWidth = (Screen.width - 20) / 8
        const maxHeight = (Screen.height - 20) / 8
        return Math.min(maxWidth, maxHeight, 64)
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
                implicitWidth: 25
                implicitHeight: 25
                imageSource: "images/bx-icon-menu.svg"
                onClicked: settingsMenu.open()
            }
        }
    }

    Root {
        id: root

        SettingsMenu {
            id: settingsMenu
            rootWidth: root.width
            onShowNewGameDialog: root.showNewGameDialog();
        }
    }
}
