import QtQuick 2.0

Item {
    property color bgColor: "white"
    property alias boardRow: myRect.boardRow;
    property alias boardColumn: myRect.boardColumn;

    width: topWindow.squareSize
    height: topWindow.squareSize

    Rectangle {
        id: myRect
        property int boardRow: 0
        property int boardColumn: 0
        anchors.fill: parent
        focus: false
        color: activeFocus ? "lightblue" : bgColor
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            myRect.focus = !myRect.focus
        }
    }
}
