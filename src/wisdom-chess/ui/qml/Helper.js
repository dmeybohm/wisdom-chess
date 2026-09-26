.pragma library

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

function isMacOS() {
    return Qt.platform.os === "osx"
}

function zeroPad(num) {
    return num < 10 ? "0" + num : "" + num
}

function promotedRow(row) {
    return row === 7 ? 4 : row
}
