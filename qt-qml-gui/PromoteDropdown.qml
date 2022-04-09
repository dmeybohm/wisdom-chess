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
        x: destinationColumn * root.squareSize
        y: destinationRow * root.squareSize
    }

    Rectangle {
        width: root.squareSize * 2
        height: root.squareSize * 2
        color: "lightblue"
    }

    MouseArea {
        anchors.fill: parent
    }

    PromotedPieceModel {
        id: myPromotedPieceModel
    }

    Grid {
        rows: 2
        columns: 2

        Repeater {
            model: myPromotedPieceModel
            delegate: Item {
               width: root.squareSize
               height: root.squareSize

               Rectangle {
                   id: myRect
                   anchors.fill: parent
                   focus: false
                   color: activeFocus ? "lightblue" : "lightsteelblue"
               }

               Image {
                   source: model.whiteImage
                   width: root.squareSize
                   height: root.squareSize
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
