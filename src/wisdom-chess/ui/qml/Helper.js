.pragma library

function zeroPad(num) {
    return num < 10 ? "0" + num : "" + num
}

function promotedRow(row) {
    return row === 7 ? 4 : row
}
