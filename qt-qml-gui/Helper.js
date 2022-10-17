
function isMobile() {
    switch (Qt.platform.os) {
    case "android":
        return true
    }
    return false
}

function isWebAssembly() {
    return Qt.platform.os == "wasm"
}

function computerOrHumanLabel(x)
{
    return x ? "Computer" : "Human";
}

function zeroPad(num) {
    return num < 10 ? "0" + num : "" + num
}

function targetRowOrCol(flipped, rowOrCol) {
    return flipped ? 8 - rowOrCol - 1 : rowOrCol
}

function promotedRow(flipped, row) {
    var shouldShiftRow = targetRowOrCol(flipped, 7);
    var shiftUpwardsRow = targetRowOrCol(flipped, 3);
    console.log('row: '+row)
    return row === shouldShiftRow ? shiftUpwardsRow : targetRowOrCol(flipped, row);
}

function promotedColumn(flipped, column) {
    return targetRowOrCol(flipped, column);
}
