import QtQuick 
import WisdomChess
import QtQuick.Controls 
import QtQuick.Layouts 

import "../popups"
import "../Helper.js" as Helper

ApplicationWindow {
    id: topWindow

    readonly property int boardWidth: boardDimensions.boardWidth
    readonly property int boardHeight: boardDimensions.boardHeight
    readonly property int squareSize: boardDimensions.squareSize

    readonly property bool isWebAssembly: Helper.isWebAssembly()
    readonly property bool isMobile: Helper.isMobile()
    readonly property bool isDesktop: !Helper.isMobile() && !Helper.isWebAssembly()
    readonly property bool isMacOS: Helper.isMacOS()

    width: boardWidth + 48
    height: boardHeight + 48 + 145

    visible: true
    title: qsTr("Wisdom Chess")
    color: "silver"

    readonly property bool anyPopupOpen: gameMenu.visible || root.anyDialogOpen

    onAnyPopupOpenChanged: {
        if (anyPopupOpen)
            _myGameModel.pause()
        else
            _myGameModel.unpause()
    }

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
            onShowSettingsDialog: root.showSettingsDialog()
        }

    }

    BoardDimensions {
        id: boardDimensions
    }
}
