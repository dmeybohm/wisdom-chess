import QtQuick 2.15
import wisdom.chess 1.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: topWindow

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

    function calculateMaxSquareSize() {
        const maxWidth = (Screen.width - 20) / 8
        const maxHeight = (Screen.height - 20) / 8
        return Math.min(maxWidth, maxHeight, 64)
    }

    onClosing: {
        _myGameModel.applicationExiting();
    }

    Screen.onPrimaryOrientationChanged: {
        root.squareSize = calculateMaxSquareSize()
        console.log("new square size: "+root.squareSize)
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
                onClicked: settingsMenu.visible ? settingsMenu.close() : settingsMenu.open()
            }
        }
    }

    Root {
        id: root

        SettingsMenu {
            id: settingsMenu
            z: 2
            rootWidth: root.width
            onShowNewGameDialog: root.showNewGameDialog();
        }
    }
}
