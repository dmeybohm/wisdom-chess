import QtQuick 2.0
import wisdom.chess 1.0

Item {
    property int destinationRow: 0
    property int destinationColumn: 0
    property int sourceRow: 0
    property int sourceColumn: 0

    transform: Translate {
        id: myTranslation
        x: destinationColumn * root.squareSize
        y: destinationRow * root.squareSize
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
            source: _myGameModel.currentTurn === Color.White ? "images/Chess_qlt45.svg" : "images/Chess_qdt45.svg"
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
            source: _myGameModel.currentTurn === Color.White ? "images/Chess_rlt45.svg" : "images/Chess_rdt45.svg"
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
            source: _myGameModel.currentTurn === Color.White ? "images/Chess_blt45.svg" : "images/Chess_bdt45.svg"
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
            source: _myGameModel.currentTurn === Color.White ? "images/Chess_nlt45.svg" : "images/Chess_ndt45.svg"
            width: root.squareSize
            height: root.squareSize

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    console.log('knight')
                }
            }
        }
    }

}
