import QtQuick 2.15
import QtQuick.Layouts 1.3
import wisdom.chess 1.0

Item {
    id: myGridAndPieces
    width: topWindow.boardWidth
    height: topWindow.boardHeight

    property var animateRowAndColChange: myPiecesLayer.animateRowAndColChange

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
                            ? "#fff3f3f3" : "green"
                }
            }
        }
    }

    PromoteDropdown {
        id: promotionDropDown
        visible: activeFocus
        focus: false
        z: 1
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
                column: model.column
                row: model.row
            }
        }

        function animateRowAndColChange(sourceRow, sourceCol, dstRow, dstCol) {
            console.log('animateRowAndColChange');

            promotionDropDown.focus = false
            if (_myGameModel.needsPawnPromotion(sourceRow, sourceCol, dstRow, dstCol)) {
                promotionDropDown.focus = true
                promotionDropDown.sourceRow = sourceRow
                promotionDropDown.sourceColumn = sourceCol
                promotionDropDown.destinationRow = dstRow
                promotionDropDown.destinationColumn = dstCol
                return;
            }
            if (_myGameModel.gameOverStatus === "") {
                _myGameModel.movePiece(sourceRow, sourceCol, dstRow, dstCol);
            }
        }
    }

}
