import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WisdomChess
import "../Helper.js" as Helper

Dialog {
    modal: true
    standardButtons: Dialog.Apply | Dialog.Cancel
    title: "Settings"

    onApplied: {
        _myGameModel.gameSettings = internal.myGameSettings
        _myGameModel.uiSettings = internal.myUISettings
        visible = false
    }

    onRejected: {
        visible = false
    }

    onVisibleChanged: {
        internal.resetSettings()
    }

    ColumnLayout {

        width: parent.width - 50
        anchors.centerIn: parent
        spacing: 10

        RowLayout {
            Text {
                Layout.fillWidth: true
                text: "White Player"
            }

            RowLayout {
                RadioButton {
                    text: "Human"
                    checked: internal.myGameSettings.whitePlayer === Player.Human
                    onClicked: internal.myGameSettings.whitePlayer = Player.Human
                }
                RadioButton {
                    text: "Computer"
                    checked: internal.myGameSettings.whitePlayer === Player.Computer
                    onClicked: internal.myGameSettings.whitePlayer = Player.Computer
                }
            }
        }

        RowLayout {
            Text {
                Layout.fillWidth: true
                text: "Black Player"
            }

            RowLayout {
                RadioButton {
                    text: "Human"
                    checked: internal.myGameSettings.blackPlayer === Player.Human
                    onClicked: internal.myGameSettings.blackPlayer = Player.Human
                }
                RadioButton {
                    text: "Computer"
                    checked: internal.myGameSettings.blackPlayer === Player.Computer
                    onClicked: internal.myGameSettings.blackPlayer = Player.Computer
                }
            }
        }

        RowLayout {
            Text {
                Layout.fillWidth: true
                text: "Flip Board"
            }

            CheckBox {
                checked: internal.myUISettings.flipped
                onClicked: internal.myUISettings.flipped = !internal.myUISettings.flipped
            }
        }

        RowLayout {
            Text {
                Layout.fillWidth: true
                text: "Thinking Time"
            }

            RowLayout {
                Text {
                    text: "0:" + Helper.zeroPad(thinkingTimeSlider.value.toString())
                }

                Slider {
                    id: thinkingTimeSlider
                    value: internal.myGameSettings.maxSearchTime
                    width: 150
                    stepSize: 1
                    from: 1
                    to: 30
                }
            }
        }

        RowLayout {
            Text {
                Layout.fillWidth: true
                text: "Search Depth"
            }

            RowLayout {
                Text {
                    text: internal.movesLabel(maxDepthSlider.value.toString())
                }

                Slider {
                    id: maxDepthSlider
                    value: internal.myGameSettings.maxDepth
                    width: thinkingTimeSlider.width
                    from: 1
                    to: 8
                    stepSize: 1
                }
            }
        }
    }

    QtObject {
        id: internal

        property var myUISettings
        property var myGameSettings

        function movesLabel(numMoves) {
            return parseInt(numMoves, 10) === 1 ? "1 move" : numMoves + " moves"
        }

        function resetSettings() {
            myUISettings = _myGameModel.cloneUISettings()
            myGameSettings = _myGameModel.cloneGameSettings()

        }

        Component.onCompleted: {
            resetSettings()
        }
    }
}

