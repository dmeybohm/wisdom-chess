import QtQuick 2.15
import wisdom.chess 1.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: topWindow

    readonly property int boardWidth: boardDimensions.boardWidth
    readonly property int boardHeight: boardDimensions.boardHeight
    readonly property int squareSize: boardDimensions.squareSize

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
        console.log("new square size: "+boardDimensions.squareSize)
    }

    header: ToolBar {
        id: toolbar
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
            ImageToolButton {
                Layout.alignment: Qt.AlignRight;
                Layout.fillHeight: true
                Layout.rightMargin: 10
                implicitWidth: 25
                implicitHeight: 25
                imageSource: "images/bx-icon-menu-white.png"
                onClicked: settingsMenu.visible ? settingsMenu.close() : settingsMenu.open()
            }
        }
    }

    MobileRoot {
        id: root
        toolbarHeight: toolbar.height
        anchors.fill: parent

        Flickable {

            visible: settingsMenu.visible
            width: settingsMenu.width
            height: Math.min(Screen.height, settingsMenu.height)

            SettingsMenu {
                id: settingsMenu
                rootWidth: root.width
                onShowNewGameDialog: root.showNewGameDialog();
            }
        }
    }

    BoardDimensions {
        id: boardDimensions
    }
}
