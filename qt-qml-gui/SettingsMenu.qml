import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "Helper.js" as Helper

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
        text: "White Player - <b>" +
              Helper.computerOrHumanLabel(_myGameModel.whiteIsComputer) +
              "</b>"
        onClicked: {
            _myGameModel.whiteIsComputer = !_myGameModel.whiteIsComputer
        }
    }
    MenuItem {
        text: "Black Player - <b>" +
              Helper.computerOrHumanLabel(_myGameModel.blackIsComputer) +
              "</b>"
        onClicked: {
            _myGameModel.blackIsComputer = !_myGameModel.blackIsComputer
        }
    }
    MenuSeparator {}

    MenuItem {
        id: thinkingTimeSliderItem
        text: "Thinking time per move"

        Text {
            text: thinkingTimeSlider.value.toString()
            anchors.right: thinkingTimeSlider.left
            anchors.verticalCenter: thinkingTimeSliderItem.verticalCenter
        }

        Slider {
            id: thinkingTimeSlider
            visible: settingsMenu.width >= settingsMenu.implicitWidth
            width: 150
            anchors {
                right: thinkingTimeSliderItem.right
                verticalCenter: thinkingTimeSliderItem.verticalCenter
            }
            stepSize: 1
            from: 1
            to: 30
        }
    }

    MenuItem {
        id: maxDepthItem
        text: "Max depth to search"

        Text {
            text: maxDepthSlider.value.toString()
            anchors.right: maxDepthSlider.left
            anchors.verticalCenter: maxDepthItem.verticalCenter
        }

        Slider {
            id: maxDepthSlider
            visible: thinkingTimeSlider.visible
            width: thinkingTimeSlider.width
            anchors {
                right: maxDepthItem.right
                verticalCenter: maxDepthItem.verticalCenter
            }
            from: 1
            to: 16
            stepSize: 1
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
