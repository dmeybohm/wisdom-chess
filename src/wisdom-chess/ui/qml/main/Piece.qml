import QtQuick

Image {
    id: myPieceImage
    width: topWindow.squareSize
    height: topWindow.squareSize

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
            x: myPieceImage.column * topWindow.squareSize
            y: myPieceImage.row * topWindow.squareSize

            Behavior on y {
                enabled: !myPieceImage.isCastlingRook && !castlingRookAnimation.running
                NumberAnimation {
                    easing.type: Easing.OutExpo
                    duration: root.animationDelay
                }
            }
            Behavior on x {
                enabled: !myPieceImage.isCastlingRook && !castlingRookAnimation.running
                NumberAnimation {
                    easing.type: Easing.OutExpo
                    duration: root.animationDelay
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
                    duration: root.animationDelay * .5
                }
            }
        }
    ]

    SequentialAnimation {
        id: castlingRookAnimation
        running: false

        PauseAnimation {
            duration: root.castlingRookPause
        }
        NumberAnimation {
            target: myTranslation
            property: "x"
            to: myPieceImage.column * topWindow.squareSize
            easing.type: Easing.OutExpo
            duration: root.animationDelay
        }
    }

    Connections {
        target: castlingRookAnimation
        function onStopped() {
            myPieceImage.rebindX()
        }
    }

    function rebindX() {
        myTranslation.x = Qt.binding(
            function() { return myPieceImage.column * topWindow.squareSize }
        )
    }

    onIsCastlingRookChanged: {
        // The role is cleared again by the next move; only its start matters.
        if (!isCastlingRook)
            return

        // break the binding so we can set an absolute start
        myTranslation.x = myTranslation.x   // this no-ops the current value but detaches a binding if any
        myTranslation.x = castlingSourceColumn * topWindow.squareSize
        castlingRookAnimation.stop()
        castlingRookAnimation.start()
    }

}
