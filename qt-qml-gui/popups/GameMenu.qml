import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../Helper.js" as Helper

Menu {
    id: gameMenu
    implicitWidth: 380

    signal showNewGameDialog()
    signal showAboutDialog()
    signal showSettingsDialog()
    signal quit()

    property bool hideFinalItem: topWindow.isWebAssembly || topWindow.isMobile
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

    MenuSeparator {
        id: finalSeparator
        height: hideFinalItem ? 0 : implicitHeight
        visible: hideFinalItem ? false : true
    }

    MenuItem {
        id: quitItem
        text: "Quit"
        height: hideFinalItem ? 0 : implicitHeight
        visible: hideFinalItem ? false : true
        onClicked: {
            gameMenu.quit()
        }
    }
}
