import QtQuick
import QtQuick.Controls

Menu {
    id: gameMenu
    implicitWidth: 380

    signal showNewGameDialog()
    signal showAboutDialog()
    signal showSettingsDialog()
    signal quit()

    readonly property bool hideFinalItem: topWindow.isWebAssembly || topWindow.isMobile
    MenuItem {
        text: "New Game"
        onClicked: {
            gameMenu.showNewGameDialog()
        }
    }

    MenuItem {
        text: "Settings"
        onClicked: {
            gameMenu.showSettingsDialog()
        }
    }

    MenuItem {
        text: "About Wisdom Chess"
        onClicked: {
            gameMenu.showAboutDialog()
        }
    }

    MenuItem {
        text: "React Version"
        height: topWindow.isWebAssembly ? implicitHeight : 0
        visible: topWindow.isWebAssembly
        onClicked: {
            Qt.openUrlExternally(_myGameModel.browserOriginUrl() + "/")
        }
    }

    MenuSeparator {
        id: finalSeparator
        height: gameMenu.hideFinalItem ? 0 : implicitHeight
        visible: !gameMenu.hideFinalItem
    }

    MenuItem {
        id: quitItem
        text: "Quit"
        height: gameMenu.hideFinalItem ? 0 : implicitHeight
        visible: !gameMenu.hideFinalItem
        onClicked: {
            gameMenu.quit()
        }
    }
}
