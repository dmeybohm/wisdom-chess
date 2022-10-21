import QtQuick 
import QtQuick.Controls 
import QtQuick.Layouts 
import WisdomChess

import "Helper.js" as Helper

Menu {
    id: settingsMenu
    implicitWidth: 380

    signal showNewGameDialog()
    signal showAboutDialog()

    MenuItem {
        text: "New Game"
        onClicked: {
            settingsMenu.showNewGameDialog()
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
                checked: _myGameModel.uiSettings.whitePlayer === Player.Human
                onClicked: _myGameModel.uiSettings.whitePlayer = Player.Human
            }
            RadioButton {
                text: "Computer"
                checked: _myGameModel.uiSettings.whitePlayer === Player.Computer
                onClicked: _myGameModel.uiSettings.whitePlayer = Player.Computer
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
                checked: _myGameModel.uiSettings.blackPlayer === Player.Human
                onClicked: _myGameModel.uiSettings.blackPlayer = Player.Human
            }
            RadioButton {
                text: "Computer"
                checked: _myGameModel.uiSettings.blackPlayer === Player.Computer
                onClicked: _myGameModel.uiSettings.blackPlayer = Player.Computer
            }
        }
    }

    MenuItem {
        text: "Flip Board"

        RowLayout {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.rightMargin: 10
            CheckBox {
                checked: _myGameModel.uiSettings.flipped
                onClicked: _myGameModel.uiSettings.flipped = !_myGameModel.uiSettings.flipped
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
            anchors.rightMargin: 5
            anchors.verticalCenter: thinkingTimeSliderItem.verticalCenter
            visible: internal.menuIsFullyVisible
        }

        Slider {
            id: thinkingTimeSlider
            value: _myGameModel.uiSettings.maxSearchTime
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
                _myGameModel.uiSettings.maxSearchTime = parseInt(value, 10)
            }
        }
    }

    MenuItem {
        id: maxDepthItem
        text: "Max depth"

        Text {
            text: internal.movesLabel(maxDepthSlider.value.toString())
            anchors.right: maxDepthSlider.left
            anchors.rightMargin: 5
            anchors.verticalCenter: maxDepthItem.verticalCenter
            visible: internal.menuIsFullyVisible
        }

        Slider {
            id: maxDepthSlider
            visible: internal.menuIsFullyVisible
            value: _myGameModel.uiSettings.maxDepth
            width: thinkingTimeSlider.width
            anchors {
                right: maxDepthItem.right
                verticalCenter: maxDepthItem.verticalCenter
            }
            from: 1
            to: 8
            stepSize: 1
            onValueChanged: {
                _myGameModel.uiSettings.maxDepth = parseInt(value, 10)
            }
        }
    }

    MenuSeparator {}

    MenuItem {
        text: "About"
        onClicked: {
            settingsMenu.showAboutDialog()
        }
    }

    QtObject {
        id: internal
        property bool menuIsFullyVisible: settingsMenu.width >= settingsMenu.implicitWidth

        function movesLabel(numMoves) {
            return parseInt(numMoves, 10) === 1 ? "1 move" : numMoves + " moves"
        }
    }
}
