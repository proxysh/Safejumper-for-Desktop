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
import QtQuick.Layouts 1.11
import QtGraphicalEffects 1.0

Column {
    id: inputPopup
    anchors.verticalCenter: parent.verticalCenter
    anchors.horizontalCenter: parent.horizontalCenter
    spacing: 8

    property string title
    property string subtitle
    property string value

    signal cancel()
    signal inputSaved(int value)

    ShadowRect {
        width: 335
        // Add 20 for top margin and 20 for bottom margin
        height: inputColumn.childrenRect.height + 20 + 20
        color: "white"
        radius: 5

        Column {
            id: inputColumn
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.topMargin: 20
            anchors.top: parent.top
            spacing: 0

            Text {
                font.family: "Roboto"
                font.bold: true
                font.pixelSize: 20
                color: defaultColor
                text: inputPopup.title
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
            }

            Item {
                height: 16
                width: parent.width
            }

            Text {
                font.family: "Roboto"
                font.bold: true
                font.pixelSize: 16
                width: 295
                wrapMode: Text.WordWrap
                color: "#6C798F"
                text: inputPopup.subtitle
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
            }

            Item {
                height: 24
                width: parent.width
            }

            HintedTextField {
                id: inputBox
                width: 295
                height: 56
                hint: qsTr("PORT");
                value: inputPopup.value
            }
        }
    } // End of rect

    ShadowRect {
        id: saveButton
        width: 335
        height: 48
        color: "#2CC532"
        radius: 5

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            font.family: "Roboto"
            font.bold: true
            font.pixelSize: 16
            text: qsTr("Save");
            color: "white"
        }

        MouseArea {
            cursorShape: Qt.PointingHandCursor
            anchors.fill: parent
            onClicked: {
                inputPopup.inputSaved(inputBox.value);
            }
        }
    }

    ShadowRect {
        id: cancelButton
        width: 335
        height: 48
        color: "#97A0AF"
        radius: 5

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            font.family: "Roboto"
            font.bold: true
            font.pixelSize: 16
            text: qsTr("Cancel");
            color: "white"
        }

        MouseArea {
            cursorShape: Qt.PointingHandCursor
            anchors.fill: parent
            onClicked: { inputPopup.cancel(); }
        }
    }

} // End of Item
