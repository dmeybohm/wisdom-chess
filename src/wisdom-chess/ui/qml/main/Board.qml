import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import WisdomChess
import "../popups"
import "../Helper.js" as Helper

Item {
    id: myGridAndPieces
    width: topWindow.boardWidth
    height: topWindow.boardHeight

    property var animateRowAndColChange: myPiecesLayer.animateRowAndColChange
    property bool flipped: _myGameModel.uiSettings.flipped

    transform: Rotation {
        origin.x: width / 2
        origin.y: height / 2
        angle: flipped ? 180 : 0

        Behavior on angle {
            NumberAnimation {
                easing.type: Easing.OutExpo
                duration: root.animationDelay * 5
            }
        }
    }

    function onFocusObjectChanged(oldObject, newObject) {
        if (oldObject && newObject &&
                'boardRow' in oldObject && 'boardRow' in newObject
        ) {
            myPiecesLayer.animateRowAndColChange(
                oldObject.boardRow,
                oldObject.boardColumn,
                newObject.boardRow,
                newObject.boardColumn
            )
            newObject.focus = false
        }
    }

    Grid {
        id: squareBackground
        width: topWindow.boardWidth
        height: topWindow.boardHeight
        columns: 8
        rows: 8
        columnSpacing: 0
        rowSpacing: 0

        Repeater {
            model: 64
            delegate: ChessSquare {
                boardRow: Math.floor(model.index / 8)
                boardColumn: model.index % 8
                bgColor: {
                    (model.index % 2 + boardRow % 2) % 2 == 0
                            ? "#fff3f3f3" : "#FF5F9EA0"
                }
            }
        }
    }

    PromoteDropdown {
        id: promotionDropDown
        visible: activeFocus
        focus: false
        z: 1
        flipped: myGridAndPieces.flipped
    }

    //
    // Layer with al the pieces:
    //
    Item {
        id: myPiecesLayer
        x: 0
        y: 0
        width: topWindow.boardWidth; height: topWindow.boardHeight

        // Pieces on top of the squares:
        Repeater {
            model: _myPiecesModel
            delegate: Piece {
                source: model.pieceImage
                row: model.row
                column: model.column
                flipped: myGridAndPieces.flipped
                isCastlingRook: model.isCastlingRook
                castlingSourceColumn: model.castlingSourceColumn
            }
        }

        function animateRowAndColChange(sourceRow, sourceCol, dstRow, dstCol) {
            promotionDropDown.focus = false
            if (_myGameModel.needsPawnPromotion(sourceRow, sourceCol, dstRow, dstCol)) {
                promotionDropDown.focus = true
                promotionDropDown.sourceRow = sourceRow
                promotionDropDown.sourceColumn = sourceCol
                promotionDropDown.destinationRow = dstRow
                promotionDropDown.destinationColumn = dstCol
                promotionDropDown.drawAtRow = Helper.promotedRow(dstRow)
                promotionDropDown.drawAtColumn = dstCol
                return;
            }
            if (_myGameModel.gameOverStatus === "") {
                _myGameModel.movePiece(sourceRow, sourceCol, dstRow, dstCol);
            }
        }
    }

}
