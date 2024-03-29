import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import WisdomChess
import "../Helper.js" as Helper

Dialog {
    modal: true
    standardButtons: Dialog.Apply | Dialog.Cancel
    title: "Settings"
    rightPadding: 25

    readonly property int indicatorOffset: topWindow.isMacOS ? 2 : 6

    onApplied: {
        applySettingsTimer.start()
        visible = false
    }

    onRejected: {
        visible = false
    }

    onVisibleChanged: {
        internal.toSaveUISettings = internal.myUISettings
        internal.toSaveGameSettings = internal.myGameSettings
        internal.resetSettings()
    }

    Component.onCompleted: {
        internal.resetSettings()
        if (Helper.isMobile()) {
            width = Screen.width - 20
            contentColumn.width = width - 30
        } else if (Helper.isWebAssembly()) {
            width = Math.min(Screen.width - 20, 500)
            contentColumn.width = width - 60
            topPadding = 30
            bottomPadding = 30
        } else {
            width = Math.min(Screen.width - 20, 400)
            contentColumn.width = width - 60
            topPadding = 30
            bottomPadding = 30
        }
    }

    Timer {
        id: applySettingsTimer
        interval: 350
        repeat: false
        onTriggered: {
            internal.applySettings()
        }
    }

    QtObject {
        id: internal

        property var myUISettings
        property var myGameSettings
        property var toSaveUISettings
        property var toSaveGameSettings

        property var fontSize: topWindow.isMobile ? "12" : "16"

        function movesLabel(numMoves) {
            return parseInt(numMoves, 10) === 1 ? "1 move" : numMoves + " moves"
        }

        function resetSettings() {
            myUISettings = _myGameModel.cloneUISettings()
            myGameSettings = _myGameModel.cloneGameSettings()
        }

        function applySettings() {
            _myGameModel.uiSettings = toSaveUISettings
            _myGameModel.gameSettings = toSaveGameSettings
        }

        Component.onCompleted: {
            resetSettings()
        }
    }

    ColumnLayout {
        id: contentColumn
        spacing: 20
        anchors.centerIn: parent

        RowLayout {
            Text {
                Layout.fillWidth: true
                font.pixelSize: internal.fontSize
                text: "White Player"
            }

            // Use Row instead of RowLayout to avoid alignment issues on desktop:
            RowLayout {
                spacing: topWindow.isDesktop ? 15 : 0
                Layout.alignment: Qt.AlignVCenter

                RadioButton {
                    text: "Human"
                    indicator.y: indicatorOffset
                    font.pixelSize: internal.fontSize
                    checked: internal.myGameSettings.whitePlayer === Player.Human
                    onClicked: internal.myGameSettings.whitePlayer = Player.Human
                }
                RadioButton {
                    text: "Computer"
                    indicator.y: indicatorOffset
                    font.pixelSize: internal.fontSize
                    checked: internal.myGameSettings.whitePlayer === Player.Computer
                    onClicked: internal.myGameSettings.whitePlayer = Player.Computer
                }
            }
        }

        RowLayout {
            Text {
                Layout.fillWidth: true
                text: "Black Player"
                font.pixelSize: internal.fontSize
            }

            RowLayout {
                spacing: topWindow.isDesktop ? 15 : 0
                Layout.alignment: Qt.AlignVCenter

                RadioButton {
                    text: "Human"
                    indicator.y: indicatorOffset
                    font.pixelSize: internal.fontSize
                    checked: internal.myGameSettings.blackPlayer === Player.Human
                    onClicked: internal.myGameSettings.blackPlayer = Player.Human
                }
                RadioButton {
                    text: "Computer"
                    indicator.y: indicatorOffset
                    font.pixelSize: internal.fontSize
                    checked: internal.myGameSettings.blackPlayer === Player.Computer
                    onClicked: internal.myGameSettings.blackPlayer = Player.Computer
                }
            }
        }

        RowLayout {
            Text {
                Layout.fillWidth: true
                text: "Flip Board"
                font.pixelSize: internal.fontSize
            }

            CheckBox {
                font.pixelSize: internal.fontSize
                checked: internal.myUISettings.flipped
                onClicked: internal.myUISettings.flipped = !internal.myUISettings.flipped
            }
        }

        RowLayout {
            Text {
                Layout.fillWidth: true
                text: "Thinking Time"
                font.pixelSize: internal.fontSize
            }

            RowLayout {
                Text {
                    text: "0:" + Helper.zeroPad(thinkingTimeSlider.value.toString())
                    font.pixelSize: internal.fontSize
                }

                Slider {
                    id: thinkingTimeSlider
                    font.pixelSize: internal.fontSize
                    implicitWidth: 150
                    value: internal.myGameSettings.maxSearchTime
                    stepSize: 1
                    from: 1
                    to: 30
                    onValueChanged: {
                        if (value && internal.myGameSettings) {
                            internal.myGameSettings.maxSearchTime = value
                        }
                    }
                }
            }
        }

        RowLayout {
            spacing: 5

            Text {
                Layout.fillWidth: true
                text: "Search Depth"
                font.pixelSize: internal.fontSize
            }

            RowLayout {
                Text {
                    text: internal.movesLabel(maxDepthSlider.value.toString())
                    font.pixelSize: internal.fontSize
                }

                Slider {
                    id: maxDepthSlider
                    font.pixelSize: internal.fontSize
                    value: internal.myGameSettings.maxDepth
                    implicitWidth: thinkingTimeSlider.implicitWidth
                    from: 1
                    to: 8
                    stepSize: 1
                    onValueChanged: {
                        if (value && internal.myGameSettings) {
                            internal.myGameSettings.maxDepth = parseInt(value, 10)
                        }
                    }

                }
            }
        }
    }


}

