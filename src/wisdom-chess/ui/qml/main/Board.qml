pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: myGridAndPieces
    width: BoardDimensions.boardWidth
    height: BoardDimensions.boardHeight

    property bool flipped: GameModel.uiSettings.flipped

    transform: Rotation {
        origin.x: myGridAndPieces.width / 2
        origin.y: myGridAndPieces.height / 2
        angle: myGridAndPieces.flipped ? 180 : 0

        Behavior on angle {
            NumberAnimation {
                easing.type: Easing.OutExpo
                duration: GameModel.animationDelay * 5
            }
        }
    }

    // The selected square, or -1. A click selects, a second click on the
    // same square deselects, and a click on another square moves. The
    // selected square has the focus, which is its highlight; a dialog that
    // takes the focus gives it back to it. Nothing is selected after a
    // move, so the destination is not focused when a draw offer opens.
    property int selectedRow: -1
    property int selectedColumn: -1

    function squareClicked(row: int, column: int): void {
        if (selectedRow < 0) {
            selectedRow = row
            selectedColumn = column
            return
        }

        const sourceRow = selectedRow
        const sourceColumn = selectedColumn
        selectedRow = -1
        selectedColumn = -1

        if (sourceRow !== row || sourceColumn !== column)
            myPiecesLayer.animateRowAndColChange(sourceRow, sourceColumn, row, column)
    }

    Grid {
        id: squareBackground
        width: BoardDimensions.boardWidth
        height: BoardDimensions.boardHeight
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
                focus: square.boardRow === myGridAndPieces.selectedRow
                    && square.boardColumn === myGridAndPieces.selectedColumn
                onClicked: myGridAndPieces.squareClicked(square.boardRow, square.boardColumn)
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
        width: BoardDimensions.boardWidth; height: BoardDimensions.boardHeight

        // Pieces on top of the squares:
        Repeater {
            model: PiecesModel
            delegate: Piece {
                flipped: myGridAndPieces.flipped
            }
        }

        function animateRowAndColChange(sourceRow: int, sourceCol: int, dstRow: int, dstCol: int): void {
            promotionDropDown.focus = false
            if (GameModel.needsPawnPromotion(sourceRow, sourceCol, dstRow, dstCol)) {
                promotionDropDown.focus = true
                promotionDropDown.sourceRow = sourceRow
                promotionDropDown.sourceColumn = sourceCol
                promotionDropDown.destinationRow = dstRow
                promotionDropDown.destinationColumn = dstCol
                // The list hangs down from the target square, or up from
                // it on the far rank.
                promotionDropDown.drawAtRow = dstRow === 7 ? 4 : dstRow
                promotionDropDown.drawAtColumn = dstCol
                return;
            }
            if (GameModel.gameOverStatus === "") {
                GameModel.movePiece(sourceRow, sourceCol, dstRow, dstCol);
            }
        }
    }

}
