import QtQuick

Image {
    id: myPieceImage
    width: BoardDimensions.squareSize
    height: BoardDimensions.squareSize

    required property string pieceImage
    required property int row
    required property int column
    required property bool isCastlingRook
    required property int castlingSourceColumn
    property bool flipped: false

    // Set while the board drags the piece: it is drawn where the board
    // puts it, without the move animation, until it is dropped.
    property bool dragging: false

    source: pieceImage

    transform: [
        Translate {
            id: myTranslation
            x: myPieceImage.column * BoardDimensions.squareSize
            y: myPieceImage.row * BoardDimensions.squareSize

            Behavior on y {
                enabled: !myPieceImage.isCastlingRook && !castlingRookAnimation.running
                    && !myPieceImage.dragging
                NumberAnimation {
                    easing.type: Easing.OutExpo
                    duration: GameModel.animationDelay
                }
            }
            Behavior on x {
                enabled: !myPieceImage.isCastlingRook && !castlingRookAnimation.running
                    && !myPieceImage.dragging
                NumberAnimation {
                    easing.type: Easing.OutExpo
                    duration: GameModel.animationDelay
                }
            }
        },

        Rotation {
            origin.x: myPieceImage.width / 2 + myTranslation.x
            origin.y: myPieceImage.height / 2 + myTranslation.y
            angle: myPieceImage.flipped ? 180 : 0

            Behavior on angle {
                NumberAnimation {
                    easing.type: Easing.OutExpo
                    duration: GameModel.animationDelay * .5
                }
            }
        }
    ]

    function lift(): void {
        dragging = true
    }

    // Draws the piece with its top left corner here, in the layer.
    function dragTo(x: real, y: real): void {
        myTranslation.x = x
        myTranslation.y = y
    }

    // The model decides where the piece belongs, and restoring the
    // bindings animates it there: to its new square, or back home when
    // the move was refused.
    function drop(): void {
        dragging = false
        rebindX()
        rebindY()
    }

    SequentialAnimation {
        id: castlingRookAnimation
        running: false
        onStopped: myPieceImage.rebindX()

        PauseAnimation {
            duration: GameModel.castlingRookPause
        }
        NumberAnimation {
            target: myTranslation
            property: "x"
            to: myPieceImage.column * BoardDimensions.squareSize
            easing.type: Easing.OutExpo
            duration: GameModel.animationDelay
        }
    }

    function rebindX(): void {
        myTranslation.x = Qt.binding(
            function(): real { return myPieceImage.column * BoardDimensions.squareSize }
        )
    }

    function rebindY(): void {
        myTranslation.y = Qt.binding(
            function(): real { return myPieceImage.row * BoardDimensions.squareSize }
        )
    }

    onIsCastlingRookChanged: {
        // The role is cleared again by the next move; only its start matters.
        if (!isCastlingRook)
            return

        // Assigning a value replaces the binding, so the rook starts from
        // its old square; rebindX() restores the binding when it arrives.
        myTranslation.x = myPieceImage.castlingSourceColumn * BoardDimensions.squareSize
        castlingRookAnimation.stop()
        castlingRookAnimation.start()
    }

}
