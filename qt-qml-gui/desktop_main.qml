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
                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter
            }

            Item {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft;
            }

            ToolButton {
                Layout.alignment: Qt.AlignRight;
                font.pointSize: 14
                text: qsTr("⋮")
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
