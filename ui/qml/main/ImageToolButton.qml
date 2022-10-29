import QtQuick
import QtQuick.Controls 

ToolButton {
    property alias imageSource: toolImage.source

    Image {
        id: toolImage
        fillMode: Image.PreserveAspectFit
        anchors.fill: parent
        anchors.margins: 2
    }
}
