import QtQuick
import QtQuick.Controls

MainWindow {
    width: BoardDimensions.boardWidth + 48
    height: BoardDimensions.boardHeight + 48 + 145
    gameRoot: root
    menu: settingsMenu

    header: ToolBar {
        // The icon, the title and the arrow all open the menu.
        Row {
            anchors.centerIn: parent
            spacing: 4

            ImageToolButton {
                id: rookButton
                implicitWidth: 32
                implicitHeight: 32
                anchors.verticalCenter: parent.verticalCenter
                imageSource: "../images/Chess_rlt45.svg"
                onClicked: settingsMenu.open()

                GameMenu {
                    id: settingsMenu
                    y: rookButton.height
                    x: -implicitWidth / 4
                    onShowAboutDialog: root.dialogs.showAboutDialog()
                    onShowNewGameDialog: root.dialogs.showNewGameDialog()
                    onShowSettingsDialog: root.dialogs.showSettingsDialog()
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
                anchors.verticalCenter: parent.verticalCenter
                implicitWidth: 15
                implicitHeight: 15
                imageSource: "../images/bxs-down-arrow.svg"
                onClicked: settingsMenu.open()
            }
        }
    }

    DesktopRoot {
        id: root
    }
}
