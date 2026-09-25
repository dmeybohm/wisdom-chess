import QtQuick
import QtQuick.Layouts

GameRoot {
    board: boards

    ColumnLayout {
        anchors.top: parent.top
        anchors.topMargin: 25
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 25

        Board {
            id: boards

            Layout.alignment: Qt.AlignHCenter
        }

        StatusBar {
            Layout.fillWidth: true
            Layout.minimumHeight: 50
        }
    }
}
