
function isMobile() {
    switch (Qt.platform.os) {
    case "android":
        return true
    }
    return false
}
