import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: settingsDialog
    modal: true
    standardButtons: Dialog.Apply | Dialog.Cancel
    title: "Settings"
    rightPadding: 25

    readonly property int indicatorOffset: Platform.isMacOS ? 2 : 6

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

    width: Platform.isMobile
        ? Screen.width - 20
        : Math.min(Screen.width - 20, Platform.isWebAssembly ? 500 : 400)
    topPadding: Platform.isMobile ? padding : 30
    bottomPadding: Platform.isMobile ? padding : 30

    Component.onCompleted: internal.resetSettings()

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

        property uiSettings myUISettings
        property gameSettings myGameSettings
        property uiSettings toSaveUISettings
        property gameSettings toSaveGameSettings

        readonly property int fontSize: Platform.isMobile ? 12 : 16

        function movesLabel(numMoves: int): string {
            return numMoves === 1 ? "1 move" : numMoves + " moves"
        }

        function zeroPad(num: int): string {
            return num < 10 ? "0" + num : "" + num
        }

        function resetSettings(): void {
            myUISettings = GameModel.cloneUISettings()
            myGameSettings = GameModel.cloneGameSettings()
        }

        function applySettings(): void {
            GameModel.uiSettings = toSaveUISettings
            GameModel.gameSettings = toSaveGameSettings
        }

        Component.onCompleted: {
            resetSettings()
        }
    }

    ColumnLayout {
        id: contentColumn
        spacing: 20
        anchors.centerIn: parent
        width: settingsDialog.width - (Platform.isMobile ? 30 : 60)

        RowLayout {
            Text {
                Layout.fillWidth: true
                font.pixelSize: internal.fontSize
                text: "White Player"
            }

            // Use Row instead of RowLayout to avoid alignment issues on desktop:
            RowLayout {
                spacing: Platform.isDesktop ? 15 : 0
                Layout.alignment: Qt.AlignVCenter

                RadioButton {
                    text: "Human"
                    indicator.y: settingsDialog.indicatorOffset
                    font.pixelSize: internal.fontSize
                    checked: internal.myGameSettings.whitePlayer === Player.Human
                    onClicked: internal.myGameSettings.whitePlayer = Player.Human
                }
                RadioButton {
                    text: "Computer"
                    indicator.y: settingsDialog.indicatorOffset
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
                spacing: Platform.isDesktop ? 15 : 0
                Layout.alignment: Qt.AlignVCenter

                RadioButton {
                    text: "Human"
                    indicator.y: settingsDialog.indicatorOffset
                    font.pixelSize: internal.fontSize
                    checked: internal.myGameSettings.blackPlayer === Player.Human
                    onClicked: internal.myGameSettings.blackPlayer = Player.Human
                }
                RadioButton {
                    text: "Computer"
                    indicator.y: settingsDialog.indicatorOffset
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
                text: "Debug Logging"
                font.pixelSize: internal.fontSize
            }

            CheckBox {
                font.pixelSize: internal.fontSize
                checked: internal.myGameSettings.debugLogging
                onClicked: internal.myGameSettings.debugLogging = !internal.myGameSettings.debugLogging
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
                    text: "0:" + internal.zeroPad(thinkingTimeSlider.value)
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
                    // moved, not valueChanged: the value is bound to the
                    // setting, so writing it back on every change loops.
                    onMoved: internal.myGameSettings.maxSearchTime = value
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
                    text: internal.movesLabel(maxDepthSlider.value)
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
                    onMoved: internal.myGameSettings.maxDepth = value
                }
            }
        }
    }


}

