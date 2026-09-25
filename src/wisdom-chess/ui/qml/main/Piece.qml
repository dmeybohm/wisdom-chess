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

    source: pieceImage

    transform: [
        Translate {
            id: myTranslation
            x: myPieceImage.column * BoardDimensions.squareSize
            y: myPieceImage.row * BoardDimensions.squareSize

            Behavior on y {
                enabled: !myPieceImage.isCastlingRook && !castlingRookAnimation.running
                NumberAnimation {
                    easing.type: Easing.OutExpo
                    duration: _myGameModel.animationDelay
                }
            }
            Behavior on x {
                enabled: !myPieceImage.isCastlingRook && !castlingRookAnimation.running
                NumberAnimation {
                    easing.type: Easing.OutExpo
                    duration: _myGameModel.animationDelay
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
                    duration: _myGameModel.animationDelay * .5
                }
            }
        }
    ]

    SequentialAnimation {
        id: castlingRookAnimation
        running: false
        onStopped: myPieceImage.rebindX()

        PauseAnimation {
            duration: _myGameModel.castlingRookPause
        }
        NumberAnimation {
            target: myTranslation
            property: "x"
            to: myPieceImage.column * BoardDimensions.squareSize
            easing.type: Easing.OutExpo
            duration: _myGameModel.animationDelay
        }
    }

    function rebindX() {
        myTranslation.x = Qt.binding(
            function() { return myPieceImage.column * BoardDimensions.squareSize }
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
