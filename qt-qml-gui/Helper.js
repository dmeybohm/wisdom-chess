
function isMobile() {
    switch (Qt.platform.os) {
    case "android":
        return true
    }
    return false
}

function computerOrHumanLabel(x)
{
    return x ? "Computer" : "Human";
}
