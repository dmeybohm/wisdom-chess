pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: myGridAndPieces
    width: topWindow.boardWidth
    height: topWindow.boardHeight

    property var animateRowAndColChange: myPiecesLayer.animateRowAndColChange
    property bool flipped: _myGameModel.uiSettings.flipped

    transform: Rotation {
        origin.x: myGridAndPieces.width / 2
        origin.y: myGridAndPieces.height / 2
        angle: myGridAndPieces.flipped ? 180 : 0

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
            const sourceRow = oldObject.boardRow
            const sourceColumn = oldObject.boardColumn
            const destinationRow = newObject.boardRow
            const destinationColumn = newObject.boardColumn

            // Before the move, which can open a draw offer: a dialog gives
            // the focus back to whatever had it when it opened.
            newObject.focus = false

            myPiecesLayer.animateRowAndColChange(
                sourceRow,
                sourceColumn,
                destinationRow,
                destinationColumn
            )
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
                id: square
                required property int index
                boardRow: Math.floor(square.index / 8)
                boardColumn: square.index % 8
                bgColor: (square.index + square.boardRow) % 2 == 0
                    ? "#fff3f3f3" : "#FF5F9EA0"
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
                flipped: myGridAndPieces.flipped
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
