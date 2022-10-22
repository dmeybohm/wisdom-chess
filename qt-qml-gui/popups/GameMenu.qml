import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../Helper.js" as Helper

Menu {
    id: gameMenu
    implicitWidth: 380

    signal showNewGameDialog()
    signal showAboutDialog()
    signal quit()

    MenuItem {
        text: "New Game"
        onClicked: {
            gameMenu.showNewGameDialog()
        }
    }
    MenuSeparator {}

    MenuItem {
        text: "About Wisdom Chess"
        onClicked: {
            gameMenu.showAboutDialog()
        }
    }

    MenuSeparator {
        visible: !Helper.isWebAssembly()
    }

    MenuItem {
        visible: !Helper.isWebAssembly()
        text: "Quit"
        onClicked: {
            gameMenu.quit()
        }
    }
}
