import QtQuick 2.15
import wisdom.chess 1.0

FocusScope {
    id: dropDownTop
    property int destinationRow: 0
    property int destinationColumn: 0
    property int sourceRow: 0
    property int sourceColumn: 0

    transform: Translate {
        id: myTranslation
        x: destinationColumn * topWindow.squareSize
        y: destinationRow * topWindow.squareSize
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

               Rectangle {
                   id: myRect
                   anchors.fill: parent
                   focus: false
                   color: activeFocus ? "lightblue" : "lightsteelblue"
               }

               Image {
                   source: model.whiteImage
                   width: topWindow.squareSize
                   height: topWindow.squareSize
               }

               MouseArea {
                   anchors.fill: parent

                   onClicked: {
                       if (myRect.focus) {
                           console.log('promote piece here')
                           _myGameModel.promotePiece(
                                       sourceRow, sourceColumn,
                                       destinationRow, destinationColumn, model.piece)
                           focus = false
                           dropDownTop.focus = false
                       } else {
                           myRect.focus = true;
                           console.log(model.piece)
                       }
                   }
               }
            }
        }
    }

}
