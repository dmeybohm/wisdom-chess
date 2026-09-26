import QtQuick

// A square of the board. Board selects it on a click by giving it the
// focus, which is its highlight.
Item {
    id: chessSquare

    property color bgColor: "white"
    property int boardRow: 0
    property int boardColumn: 0

    signal clicked()

    width: BoardDimensions.squareSize
    height: BoardDimensions.squareSize
    focus: false

    Rectangle {
        anchors.fill: parent
        color: chessSquare.activeFocus ? "lightblue" : chessSquare.bgColor
    }

    // A handler rather than a MouseArea: the pieces layer's DragHandler
    // above the square takes over a press that turns into a drag, and a
    // tap is what is left when it does not.
    TapHandler {
        onTapped: chessSquare.clicked()
    }
}
