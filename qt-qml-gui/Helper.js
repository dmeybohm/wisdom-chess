
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
