import QtQuick 2.0

Item {
    property int row: 0
    property int column: 0

    transform: Translate {
        id: myTranslation
        x: column * root.squareSize
        y: row * root.squareSize
    }

    Rectangle {
        width: root.squareSize * 2
        height: root.squareSize * 2
        color: "lightblue"
    }

    MouseArea {
        anchors.fill: parent
    }

    Grid {
        rows: 2
        columns: 2

        Image {
            source: _gameModel.currentTurn == Color.White ? "images/Chess_qlt45.svg" : "images/Chess_qdt45.svg"
            width: root.squareSize
            height: root.squareSize

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    console.log('queen')
                }
            }
        }
        Image {
            source: _gameModel.currentTurn == Color.White ? "images/Chess_rlt45.svg" : "images/Chess_rdt45.svg"
            width: root.squareSize
            height: root.squareSize

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    console.log('rook')
                }
            }
        }
        Image {
            source: _gameModel.currentTurn == Color.White ? "images/Chess_qlt45.svg" : "images/Chess_qdt45.svg"
            width: root.squareSize
            height: root.squareSize

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    console.log('bishop')
                }
            }
        }

        Image {
            source: _gameModel.currentTurn == Color.White ? "image/Chess_nlt45.svg" : "images/Chess_ndt45.svg"
            width: root.squareSize
            height: root.squareSize

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    console.log('night')
                }
            }
        }
    }

}
