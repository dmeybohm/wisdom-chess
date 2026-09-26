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

    // A mouse drag is the other way to move. Lifting a piece drops any
    // click selection, and the drop goes through the same path as a
    // second click, so a drop off the board or back home does nothing.
    function pieceLifted(): void {
        selectedRow = -1
        selectedColumn = -1
    }

    function pieceDropped(sourceRow: int, sourceColumn: int, row: int, column: int): void {
        if (row < 0 || row > 7 || column < 0 || column > 7)
            return
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

        property Piece dragged: null
        // Where the piece was pressed, from its top left corner.
        property real dragOffsetX: 0
        property real dragOffsetY: 0
        // The dragged piece's top left corner, kept for the drop.
        property real dragX: 0
        property real dragY: 0

        // Pieces on top of the squares:
        Repeater {
            id: piecesRepeater
            model: PiecesModel
            delegate: Piece {
                flipped: myGridAndPieces.flipped
            }
        }

        // One handler for the whole layer, which picks the piece by the
        // square under the press, so a piece still sliding into its square
        // can be picked up from there. A drag never leaves the scene, so
        // it needs no OS drag and drop and works the same on every
        // platform. A finger does not drag: its press falls through to
        // the square's tap handling.
        DragHandler {
            id: pieceDrag
            target: null
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad

            onActiveChanged: {
                if (active)
                    myPiecesLayer.lift()
                else
                    myPiecesLayer.drop()
            }

            onActiveTranslationChanged: myPiecesLayer.follow()
        }

        function pieceAt(row: int, column: int): Piece {
            for (let i = 0; i < piecesRepeater.count; i++) {
                const piece = piecesRepeater.itemAt(i) as Piece
                if (piece && piece.row === row && piece.column === column)
                    return piece
            }
            return null
        }

        // Mapping from the scene undoes the board's rotation when it is
        // flipped.
        function pointerAt(scenePosition: point): point {
            return myPiecesLayer.mapFromItem(null, scenePosition.x, scenePosition.y)
        }

        function lift(): void {
            const size = BoardDimensions.squareSize
            const pressed = pointerAt(pieceDrag.centroid.scenePressPosition)
            const row = Math.floor(pressed.y / size)
            const column = Math.floor(pressed.x / size)
            if (!GameModel.canMoveFrom(row, column))
                return

            dragged = pieceAt(row, column)
            if (!dragged)
                return

            dragOffsetX = pressed.x - column * size
            dragOffsetY = pressed.y - row * size
            myGridAndPieces.pieceLifted()
            dragged.lift()
            follow()
        }

        function follow(): void {
            if (!dragged)
                return

            const at = pointerAt(pieceDrag.centroid.scenePosition)
            dragX = at.x - dragOffsetX
            dragY = at.y - dragOffsetY
            dragged.dragTo(dragX, dragY)
        }

        // The piece lands on the square under its middle. The move comes
        // before the drop, so the piece animates from where it was let go
        // to wherever the model then says it is.
        function drop(): void {
            if (!dragged)
                return

            const piece = dragged
            dragged = null
            const size = BoardDimensions.squareSize
            const row = Math.floor((dragY + size / 2) / size)
            const column = Math.floor((dragX + size / 2) / size)
            myGridAndPieces.pieceDropped(piece.row, piece.column, row, column)
            piece.drop()
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
