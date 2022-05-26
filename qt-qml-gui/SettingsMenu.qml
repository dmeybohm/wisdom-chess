import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "Helper.js" as Helper

Menu {
    id: settingsMenu
    implicitWidth: 380

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
    MenuSeparator {}
    MenuItem {
        text: "White Player"

        RowLayout {
            visible: internal.menuIsFullyVisible
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.rightMargin: 10

            RadioButton {
                text: "Human"
                checked: !_myGameModel.whiteIsComputer
                onClicked: _myGameModel.whiteIsComputer = false
            }
            RadioButton {
                text: "Computer"
                checked: _myGameModel.whiteIsComputer
                onClicked: _myGameModel.whiteIsComputer = true
            }
        }
    }

    MenuItem {
        text: "Black Player"

        RowLayout {
            visible: internal.menuIsFullyVisible
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.rightMargin: 10

            RadioButton {
                text: "Human"
                checked: !_myGameModel.blackIsComputer
                onClicked: _myGameModel.blackIsComputer = false
            }
            RadioButton {
                text: "Computer"
                checked: _myGameModel.blackIsComputer
                onClicked: _myGameModel.blackIsComputer = true
            }
        }
    }

    MenuSeparator {}

    MenuItem {
        id: thinkingTimeSliderItem
        text: "Thinking time"

        Text {
            text: "0:" + Helper.zeroPad(thinkingTimeSlider.value.toString())
            anchors.right: thinkingTimeSlider.left
            anchors.verticalCenter: thinkingTimeSliderItem.verticalCenter
            visible: internal.menuIsFullyVisible
        }

        Slider {
            id: thinkingTimeSlider
            value: _myGameModel.maxSearchTime
            visible: internal.menuIsFullyVisible
            width: 150
            anchors {
                right: thinkingTimeSliderItem.right
                verticalCenter: thinkingTimeSliderItem.verticalCenter
            }
            stepSize: 1
            from: 1
            to: 30
            onValueChanged: {
                _myGameModel.maxSearchTime = parseInt(value, 10)
            }
        }
    }

    MenuItem {
        id: maxDepthItem
        text: "Max depth"

        Text {
            text: internal.movesLabel(maxDepthSlider.value.toString())
            anchors.right: maxDepthSlider.left
            anchors.verticalCenter: maxDepthItem.verticalCenter
            visible: internal.menuIsFullyVisible
        }

        Slider {
            id: maxDepthSlider
            visible: internal.menuIsFullyVisible
            value: _myGameModel.maxDepth
            width: thinkingTimeSlider.width
            anchors {
                right: maxDepthItem.right
                verticalCenter: maxDepthItem.verticalCenter
            }
            from: 1
            to: 8
            stepSize: 1
            onValueChanged: {
                _myGameModel.maxDepth = parseInt(value, 10)
            }
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

    QtObject {
        id: internal
        property bool menuIsFullyVisible: settingsMenu.width >= settingsMenu.implicitWidth

        function movesLabel(numMoves) {
            return numMoves == 1 ? "1 move" : numMoves + " moves"
        }
    }
}
