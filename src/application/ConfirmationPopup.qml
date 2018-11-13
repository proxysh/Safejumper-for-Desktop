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
    id: confirmationPopup
    anchors.bottom: parent.bottom
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.bottomMargin: 20
    spacing: 8

    property string title
    property string subtitle
    property string confirmText: qsTr("OK");
    property string cancelText: qsTr("CANCEL");

    signal cancel()
    signal confirm()

    ShadowRect {
        id: confirmationBox

        width: 335
        height: confirmationColumn.childrenRect.height + 36 + 36
        color: "white"
        radius: 5

        Column {
            id: confirmationColumn
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 36
            anchors.top: parent.top
            spacing: 8

            Text {
                font.family: "Roboto"
                font.bold: true
                font.pixelSize: 20
                color: defaultColor
                text: confirmationPopup.title
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                font.family: "Roboto"
                font.pixelSize: 18
                font.weight: Font.Medium
                color: "#6C798F"
                text: confirmationPopup.subtitle
                horizontalAlignment: Qt.AlignHCenter
                width: 335-40 // 20px margin on both sides
                wrapMode: Text.WordWrap
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

    } // End of rect

    ShadowRect {
        id: okButton
        width: 335
        height: 48
        color: defaultColor
        radius: 5

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            font.family: "Roboto"
            font.bold: true
            font.pixelSize: 16
            text: confirmationPopup.confirmText
            color: "white"
        }

        MouseArea {
            cursorShape: Qt.PointingHandCursor
            anchors.fill: parent
            onClicked: {
                confirmationPopup.confirm();
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
            text: confirmationPopup.cancelText
            color: "white"
        }

        MouseArea {
            cursorShape: Qt.PointingHandCursor
            anchors.fill: parent
            onClicked: {
                confirmationPopup.cancel();
            }
        }
    }
} // End of Item
