import QtQuick

ListModel {
    property bool reversed: false

    ListElement {
        whiteImage: "../images/Chess_qlt45.svg"
        blackImage: "../images/Chess_qdt45.svg"
        piece: 'queen'
    }
    ListElement {
        whiteImage: "../images/Chess_rlt45.svg"
        blackImage: "../images/Chess_rdt45.svg"
        piece: 'rook'
    }
    ListElement {
        whiteImage: "../images/Chess_blt45.svg"
        blackImage: ".../images/Chess_bdt45.svg"
        piece: 'bishop'
    }
    ListElement {
        whiteImage: "../images/Chess_nlt45.svg"
        blackImage: "../images/Chess_ndt45.svg"
        piece: 'knight'
    }

    function setFirstRow(firstRow) {
        if (firstRow === 7 && !reversed ||
            firstRow === 0 && reversed) {
            reverse()
        }
    }

    function swap(a, b) {
        if (a < b) {
            move(a, b, 1)
            move (b-1, a, 1)
        } else if (a > b) {
            move(b, a, 1)
            move (a-1, b, 1)
        }
    }

    function reverse() {
        for (var i = 0; i < Math.floor(count/2); ++i) {
            swap(i, count-i-1)
        }
        reversed = !reversed
    }
}
