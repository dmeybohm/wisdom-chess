import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Menu {
    id: settingsMenu
    implicitWidth: 340

    property int rootWidth
    signal showAcceptDrawDialog()
    signal showNewGameDialog()

    x: rootWidth - settingsMenu.width
    MenuItem {
        text: "New Game"
        onClicked: {
            root.showNewGameDialog()
        }
    }
    MenuItem {
        text: "Load Game from FEN"
    }
    MenuItem {
        text: "Copy FEN string for current position"
    }
    MenuSeparator {}
    MenuItem {
        text: "White Player - <b>Human</b>"
        onClicked: {
            acceptDrawDialog()
        }
    }
    MenuItem {
        text: "Black Player - <b>Computer</b>"
    }
    MenuSeparator {}

    MenuItem {
        id: thinkingTimeSliderItem
        text: "Thinking time per move"
        Slider {
            id: thinkingTimeSlider
            visible: settingsMenu.width >= settingsMenu.implicitWidth
            width: 150
            anchors {
                right: thinkingTimeSliderItem.right
                verticalCenter: thinkingTimeSliderItem.verticalCenter
            }
            from: 1
            to: 30
        }
    }

    MenuItem {
        id: maxDepthItem
        text: "Max depth to search"
        Slider {
            visible: thinkingTimeSlider.visible
            width: thinkingTimeSlider.width
            anchors {
                right: maxDepthItem.right
                verticalCenter: maxDepthItem.verticalCenter
            }
            from: 1
            to: 16
        }
    }

    enter: Transition {
        ParallelAnimation {
            NumberAnimation {
                property: "width"
                from: 0
                to: settingsMenu.implicitWidth
                easing.type: Easing.InOutExpo
                duration: 350
            }
            NumberAnimation {
                property: "height"
                from: 0
                to: settingsMenu.implicitHeight
                easing.type: Easing.InOutExpo
                duration: 350
            }
        }
    }
}
