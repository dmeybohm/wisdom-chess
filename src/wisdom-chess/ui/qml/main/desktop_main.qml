import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

MainWindow {
    width: BoardDimensions.boardWidth + 48
    height: BoardDimensions.boardHeight + 48 + 145

    header: ToolBar {
        height: 35

        RowLayout {
            anchors.fill: parent

            Label {
                visible: Platform.isWebAssembly
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
        }
    }

    DesktopRoot {
        id: root
        menu: gameMenu

        GameMenu {
            id: gameMenu
            x: 0
        }
    }
}
