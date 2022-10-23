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
    }

    MenuItem {
        id: quitItem
        text: "Quit"
        onClicked: {
            gameMenu.quit()
        }
    }

    Component.onCompleted: {
        // Hide quit option and separator on webassembly / mobile:
        if (Helper.isWebAssembly() || Helper.isMobile()) {
            finalSeparator.height = 0
            finalSeparator.visible = false
            quitItem.height = 0
            quitItem.visible = false
        }
    }
}
