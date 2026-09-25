import QtQuick

Item {
    id: chessSquare

    property color bgColor: "white"
    property alias boardRow: myRect.boardRow;
    property alias boardColumn: myRect.boardColumn;

    width: BoardDimensions.squareSize
    height: BoardDimensions.squareSize

    Rectangle {
        id: myRect
        property int boardRow: 0
        property int boardColumn: 0
        anchors.fill: parent
        focus: false
        color: activeFocus ? "lightblue" : chessSquare.bgColor
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            myRect.focus = !myRect.focus
        }
    }
}
