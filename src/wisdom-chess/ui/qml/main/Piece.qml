import QtQuick

Image {
    id: myPieceImage
    width: topWindow.squareSize
    height: topWindow.squareSize

    property int row: 0
    property int column: 0
    property bool flipped: false
    property bool isCastlingRook: false
    property int castlingSourceColumn: 0

    transform: [
        Translate {
            id: myTranslation
            x: column * topWindow.squareSize
            y: row * topWindow.squareSize

            Behavior on y {
                enabled: !myPieceImage.isCastlingRook
                NumberAnimation {
                    easing.type: Easing.OutExpo
                    duration: root.animationDelay
                }
            }
            Behavior on x {
                enabled: !myPieceImage.isCastlingRook
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
            duration: 225
        }
        NumberAnimation {
            target: myTranslation
            property: "x"
            to: myPieceImage.column * topWindow.squareSize
            easing.type: Easing.OutExpo
            duration: root.animationDelay
        }
        ScriptAction {
            script: {
                myTranslation.x = Qt.binding(
                    function() { return myPieceImage.column * topWindow.squareSize }
                )
            }
        }
    }

    onIsCastlingRookChanged: {
        if (isCastlingRook) {
            myTranslation.x = castlingSourceColumn * topWindow.squareSize
            castlingRookAnimation.restart()
        }
    }

}
