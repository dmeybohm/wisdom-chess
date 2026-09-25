import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

MainWindow {
    width: Screen.width
    height: Screen.height
    gameRoot: root
    menu: gameMenu

    Screen.onPrimaryOrientationChanged: {
        BoardDimensions.squareSize = BoardDimensions.calculateMaxSquareSize()
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
        anchors.fill: parent

        GameMenu {
            id: gameMenu
            // A press on the menu button must not close the menu, or the
            // button's click on release would open it again.
            parent: toolbar
            x: toolbar.width - gameMenu.width
            y: toolbar.height
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
            onShowNewGameDialog: root.dialogs.showNewGameDialog()
            onShowAboutDialog: root.dialogs.showAboutDialog()
            onShowSettingsDialog: root.dialogs.showSettingsDialog()
        }
    }
}
