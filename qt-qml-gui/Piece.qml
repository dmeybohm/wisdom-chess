import QtQuick 2.15

Image {
    id: myPieceImage
    width: root.squareSize
    height: root.squareSize

    property int row: 0
    property int column: 0

    transform: Translate {
        id: myTranslation
        x: column * root.squareSize
        y: row * root.squareSize

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
    }

}
