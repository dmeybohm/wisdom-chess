import QtQuick
import WisdomChess 1.0

FocusScope {
    id: dropDownTop
    property int destinationRow: 0
    property int destinationColumn: 0
    property int sourceRow: 0
    property int sourceColumn: 0
    property int drawAtRow: 0
    property int drawAtColumn: 0
    property bool flipped: false

    onDestinationColumnChanged: {
        myPromotedPieceModel.setFirstRow(destinationRow)
    }

    transform: Translate {
        id: myTranslation
        x: drawAtColumn * topWindow.squareSize
        y: drawAtRow * topWindow.squareSize
    }

    Rectangle {
        width: topWindow.squareSize
        height: topWindow.squareSize * 4
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
               width: topWindow.squareSize
               height: topWindow.squareSize

               transform: Rotation {
                    origin.x: width / 2
                    origin.y: height / 2
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
                   source: _myGameModel.currentTurn === ChessColor.White ? model.whiteImage
                                                                : model.blackImage
                   width: topWindow.squareSize
                   height: topWindow.squareSize
               }

               MouseArea {
                   anchors.fill: parent

                   onClicked: {
                       if (myRect.focus) {
                           _myGameModel.promotePiece(
                                       sourceRow, sourceColumn,
                                       destinationRow, destinationColumn, model.piece)
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
