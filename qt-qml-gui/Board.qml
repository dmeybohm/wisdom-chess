import QtQuick 2.4
import QtQuick.Layouts 1.3

Item {
    id: myGridAndPieces
    width: boardWidth
    height: boardHeight
    anchors.centerIn: parent
    property var animateRowAndColChange: myPiecesLayer.animateRowAndColChange

    function onFocusObjectChanged(oldObject, newObject) {
        console.log('oldObject:', oldObject)
        console.log('newObject:', newObject)
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
        width: parent.width
        height: parent.height
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

    //
    // Layer with al the pieces:
    //
    Item {
        id: myPiecesLayer
        x: 0
        y: 0
        width: boardWidth; height: boardHeight

        // Pieces on top of the squares:
        Repeater {
            model: _myGameModel
            delegate: Piece {
                source: model.pieceImage
                column: model.column
                row: model.row
            }
        }

        function animateRowAndColChange(sourceRow, sourceCol, dstRow, dstCol) {
            console.log('animateRowAndColChange');
            _myGameModel.movePiece(sourceRow, sourceCol, dstRow, dstCol);
        }
    }
}
