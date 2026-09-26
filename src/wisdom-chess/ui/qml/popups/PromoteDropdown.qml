pragma ComponentBehavior: Bound

import QtQuick

FocusScope {
    id: dropDownTop
    property int destinationRow: 0
    property int destinationColumn: 0
    property int sourceRow: 0
    property int sourceColumn: 0
    property int drawAtRow: 0
    property int drawAtColumn: 0
    property bool flipped: false

    // Queen first, on the target square, with the rest hanging down from
    // it. A promotion on the far rank draws the list upwards from that
    // square instead (drawAtRow), so the order is reversed to keep the
    // queen on it.
    readonly property list<int> choices: destinationRow === 7
        ? [PieceType.Knight, PieceType.Bishop, PieceType.Rook, PieceType.Queen]
        : [PieceType.Queen, PieceType.Rook, PieceType.Bishop, PieceType.Knight]

    function imageFor(piece: int): string {
        const letter = piece === PieceType.Queen ? "q"
            : piece === PieceType.Rook ? "r"
            : piece === PieceType.Bishop ? "b"
            : "n"
        const side = GameModel.currentTurn === Color.White ? "l" : "d"
        return "../images/Chess_" + letter + side + "t45.svg"
    }

    transform: Translate {
        id: myTranslation
        x: dropDownTop.drawAtColumn * BoardDimensions.squareSize
        y: dropDownTop.drawAtRow * BoardDimensions.squareSize
    }

    Rectangle {
        width: BoardDimensions.squareSize
        height: BoardDimensions.squareSize * 4
        color: "lightblue"
    }

    MouseArea {
        anchors.fill: parent
    }

    Grid {
        rows: 4
        columns: 1

        Repeater {
            model: 4

            delegate: Item {
               id: choice
               required property int index
               readonly property int piece: dropDownTop.choices[index]

               width: BoardDimensions.squareSize
               height: BoardDimensions.squareSize

               transform: Rotation {
                    origin.x: choice.width / 2
                    origin.y: choice.height / 2
                    angle: dropDownTop.flipped ? 180 : 0
                    axis.x: 1
                    axis.y: 0
                    axis.z: 0
               }

               Rectangle {
                   id: myRect
                   anchors.fill: parent
                   focus: false
                   color: activeFocus ? "lightblue" : "lightsteelblue"
               }

               Image {
                   source: dropDownTop.imageFor(choice.piece)
                   width: BoardDimensions.squareSize
                   height: BoardDimensions.squareSize
               }

               MouseArea {
                   anchors.fill: parent

                   onClicked: {
                       if (myRect.focus) {
                           GameModel.promotePiece(
                                       dropDownTop.sourceRow, dropDownTop.sourceColumn,
                                       dropDownTop.destinationRow, dropDownTop.destinationColumn,
                                       choice.piece)
                           focus = false
                           dropDownTop.focus = false
                       } else {
                           myRect.focus = true;
                       }
                   }
               }
            }
        }
    }
}
