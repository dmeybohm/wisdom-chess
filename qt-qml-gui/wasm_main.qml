import QtQuick 2.15
import wisdom.chess 1.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

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

        Row {
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter

            Row {
                spacing: 4

                ImageToolButton {
                    id: rookButton
                    implicitWidth: 32
                    implicitHeight: 32
                    anchors.verticalCenter: parent.verticalCenter
                    imageSource: "images/Chess_rlt45.svg"

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: settingsMenu.open()
                    }

                    SettingsMenu {
                        y: rookButton.height
                        x: -implicitWidth / 4
                        id: settingsMenu
                        onShowAboutDialog: root.showAboutDialog()
                        onShowNewGameDialog: root.showNewGameDialog()
                    }
                }

                Label {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    text: "Wisdom Chess"
                    verticalAlignment: Text.AlignVCenter

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: settingsMenu.open()
                    }
                }

                ImageToolButton {
                    id: menuButton
                    anchors.verticalCenter: parent.verticalCenter
                    implicitWidth: 15
                    implicitHeight: 15
                    imageSource: "images/bxs-down-arrow.svg"

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: settingsMenu.open()
                    }
                }
            }
        }
    }

    DesktopRoot {
        id: root
    }

    BoardDimensions {
        id: boardDimensions
    }
}
