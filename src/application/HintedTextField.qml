/***************************************************************************
 *   Copyright (C) 2018 by Jeremy Whiting <jeremypwhiting@gmail.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation version 2 of the License.                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

ShadowRect {
    id: hintedTextField

    property string hint

    property bool password: false
    property string value

    signal accepted()

    color: "white"
    radius: 5

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: hintedTextField.verticalCenter
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 0

        Text {
            font.pixelSize: 12
            font.family: "Roboto"
            font.weight: Font.Black
            color: "#6C798F"
            text: hint;
            Layout.preferredHeight: 12
            visible: inputBox.length > 0
            Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
        }

        TextField {
            id: inputBox
            Layout.preferredWidth: parent.width
            leftPadding: 0
            placeholderText: hint;
            font.pixelSize: 16
            color: "#97A0AF"
            font.family: "Roboto"
            font.bold: true
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            selectByMouse: true
            echoMode: (password ? TextInput.Password : TextInput.Normal)

            text: hintedTextField.value

            background: Rectangle {
                      color: "transparent"
                      border.color: "transparent"
                  }

            onTextChanged: { hintedTextField.value = text; }
            onAccepted: { hintedTextField.accepted(); }
        }
    }

    Image {
        visible: password
        anchors.right: hintedTextField.right
        anchors.rightMargin: 16
        anchors.verticalCenter: hintedTextField.verticalCenter
        source: "../images/eye.png"

        MouseArea {
            cursorShape: Qt.PointingHandCursor
            anchors.fill: parent
            onClicked: {
                if (inputBox.echoMode == TextInput.Password)
                    inputBox.echoMode = TextInput.Normal
                else
                    inputBox.echoMode = TextInput.Password
            }
        }
    }
}
