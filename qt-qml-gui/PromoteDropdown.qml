import QtQuick 2.0

Item {
    property int row: 0
    property int column: 0
    z: 1

    transform: Translate {
        id: myTranslation
        x: column * root.squareSize
        y: row * root.squareSize
    }

    Rectangle {
        width: root.squareSize * 2
        height: root.squareSize * 2
        color: "lightblue"
    }

    MouseArea {
        anchors.fill: parent
    }

    Grid {
        rows: 2
        columns: 2

        Image {
            source: "images/Chess_qdt45.svg"
            width: root.squareSize
            height: root.squareSize
        }
        Image {
            source: "images/Chess_qdt45.svg"
            width: root.squareSize
            height: root.squareSize
        }
        Image {
            source: "images/Chess_qdt45.svg"
            width: root.squareSize
            height: root.squareSize
        }
        Image {
            source: "images/Chess_qdt45.svg"
            width: root.squareSize
            height: root.squareSize
        }
    }

}
