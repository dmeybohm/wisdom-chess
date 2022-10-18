import QtQuick

Image {
    id: myPieceImage
    width: topWindow.squareSize
    height: topWindow.squareSize

    property int row: 0
    property int column: 0
    property bool flipped: false

    transform: [
        Translate {
            id: myTranslation
            x: column * topWindow.squareSize
            y: row * topWindow.squareSize

            Behavior on y {
                NumberAnimation {
                    easing.type: Easing.OutExpo
                    duration: root.animationDelay
                }
            }
            Behavior on x {
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
            axis.x: 1
            axis.y: 0
            axis.z: 0

            Behavior on angle {
                NumberAnimation {
                    easing.type: Easing.OutExpo
                    duration: root.animationDelay * .5
                }
            }
        }
    ]

}
