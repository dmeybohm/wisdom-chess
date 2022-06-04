import QtQuick 2.0
import QtQuick.Controls 2.15

ToolButton {
    property alias imageSource: toolImage.source

    Image {
        id: toolImage
        fillMode: Image.PreserveAspectFit
        anchors.fill: parent
        anchors.margins: 2
    }
}
