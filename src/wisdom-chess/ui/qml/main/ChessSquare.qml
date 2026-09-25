import QtQuick

// A square takes the focus when clicked; Board turns the focus moving from
// one square to another into a move.
Item {
    id: chessSquare

    property color bgColor: "white"
    property int boardRow: 0
    property int boardColumn: 0

    width: BoardDimensions.squareSize
    height: BoardDimensions.squareSize
    focus: false

    Rectangle {
        anchors.fill: parent
        color: chessSquare.activeFocus ? "lightblue" : chessSquare.bgColor
    }

    MouseArea {
        anchors.fill: parent
        onClicked: chessSquare.focus = !chessSquare.focus
    }
}
