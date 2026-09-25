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

    onDestinationRowChanged: {
        myPromotedPieceModel.setFirstRow(destinationRow)
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

    PromotedPieceModel {
        id: myPromotedPieceModel
    }

    Grid {
        rows: 4
        columns: 1

        Repeater {
            model: myPromotedPieceModel
            delegate: Item {
               id: choice
               required property string whiteImage
               required property string blackImage
               required property int piece

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
                   source: GameModel.currentTurn === Color.White ? choice.whiteImage
                                                                : choice.blackImage
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
