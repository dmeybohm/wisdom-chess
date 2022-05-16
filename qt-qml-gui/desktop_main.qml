import QtQuick 2.15
import wisdom.chess 1.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: topWindow

    width: root.boardWidth + 48
    height: root.boardHeight + 48 + 200

    visible: true
    title: qsTr("Wisdom Chess")
    color: "silver"

    property var currentFocusedItem: null
    onFocusObjectChanged: {
        root.onFocusObjectChanged(root.currentFocusedItem, activeFocusItem)
        root.currentFocusedItem = activeFocusItem
    }

    function calculateMaxSquareSize() {
        const maxWidth = (Screen.width - 20) / 8
        const maxHeight = (Screen.height - 20) / 8
        return Math.min(maxWidth, maxHeight, 64)
    }

    onClosing: {
        _myGameModel.applicationExiting();
    }

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            Label {
                text: "Wisdom Chess"
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.leftMargin: 17
            }
            ToolButton {
                text: qsTr("⋮")
                onClicked: contextMenu.open()
            }
        }
    }

    Root {
        id: root

        Menu {
            id: contextMenu
            x: root.width - contextMenu.width
            implicitWidth: 340

            MenuItem {
                text: "New Game"
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
                    root.acceptDrawDialog.open()
                }
            }
            MenuItem {
                text: "Black Player - <b>Computer</b>"
            }
            MenuSeparator {}

            MenuItem {
                id: thinkingTimeSlider
                text: "Thinking time per move"
                Slider {
                    anchors {
                        right: thinkingTimeSlider.right
                        verticalCenter: thinkingTimeSlider.verticalCenter
                    }
                    from: 1
                    to: 30
                }
            }

            MenuItem {
                id: maxDepthItem
                text: "Max depth to search"
                Slider {
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
                        from: 0.0
                        to: contextMenu.implicitWidth
                        easing.type: easing.InOutExpo
                        duration: 200
                    }
                    NumberAnimation {
                        property: "height"
                        from: 0.0
                        to: contextMenu.implicitHeight
                        easing.type: easing.InOutExpo
                        duration: 200
                    }
                }
            }
        }
    }
}
