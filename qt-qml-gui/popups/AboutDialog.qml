import QtQuick 
import QtQuick.Controls 
import "../Helper.js" as Helper

Dialog {
    id: aboutDialog
    modal: true
    title: "About Wisdom Chess"
    implicitWidth: topWindow.isWebAssembly ? 550 : 400
    implicitHeight: topWindow.isWebAssembly ? 385 : 350

    onAccepted: {
        visible = false
    }

    footer: DialogButtonBox {
        standardButtons: Dialog.Ok
        alignment: topWindow.isWebAssembly ? Qt.AlignHCenter : Qt.AlignRight
    }

    Column {
        anchors {
            left: parent.left;
            leftMargin: 25
            rightMargin: 25
            right: parent.right;
            top: parent.top
            topMargin: 30
        }
        spacing: 20

        Text {
            id: firstLine
            text: "Wisdom Chess © David Meybohm 2022"
            font.pointSize: 14
            width: parent.width

            verticalAlignment: topWindow.isMobile ? Text.AlignTop : Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Text {
            id: secondLine
            text: "Images © Colin M.L. Burnett and used under creative commons license."
            font.pointSize: 14
            width: parent.width

            verticalAlignment: topWindow.isMobile ? Text.AlignTop : Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Text {
            id: thirdLine
            text: "Box icons © boxicons.com and used under creative commons license."
            font.pointSize: 14
            width: parent.width

            verticalAlignment: topWindow.isMobile ? Text.AlignTop : Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Text {
            id: fourthLine
            text: "Qt © The Qt Company 2021 used under GPL / LGPL license."
            font.pointSize: 14
            width: parent.width

            verticalAlignment: topWindow.isMobile ? Text.AlignTop : Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }
}
