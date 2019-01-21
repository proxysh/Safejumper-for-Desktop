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
    id: logsPopup
    anchors.top: parent.top
    anchors.topMargin: 20
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 20
    anchors.horizontalCenter: parent.horizontalCenter
    spacing: 8

    signal copy()
    signal cancel()

    function updateLogs()
    {
        console.log("updateLogs called");
        logsText.text = mainwindow.logsContent();
    }

    ShadowRect {
        id: logsBox

        anchors.horizontalCenter: parent.horizontalCenter
        width: 335
        height: 600
        color: "white"
        radius: 5

        Column {
            id: logsColumn
            anchors.fill: parent
            anchors.topMargin: 12
            anchors.bottomMargin: 12
            spacing: 12

            Text {
                id: titleText
                font.family: "Roboto"
                font.bold: true
                font.pixelSize: 20
                color: defaultColor
                text: qsTr("Logs");
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Flickable {
                width: 295
                height: logsBox.height -titleText.height - 36
                anchors.left: parent.left
                anchors.leftMargin: 20
                contentHeight: logsText.height
                clip: true
                contentY : contentHeight-height

                Text {
                    id: logsText
                    font.family: "Roboto"
                    font.pixelSize: 10
                    color: "#6C798F"
//                    text: mainwindow.logscontent
                    horizontalAlignment: Qt.AlignLeft
                    width: 295 // 20px margin on both sides
                    wrapMode: Text.WordWrap
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    } // End of rect

    ShadowRect {
        id: copyButton
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
            text: qsTr("COPY LOGS");
            color: "white"
        }

        MouseArea {
            cursorShape: Qt.PointingHandCursor
            anchors.fill: parent
            onClicked: {
                logsPopup.copy();
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
            text: qsTr("CLOSE");
            color: "white"
        }

        MouseArea {
            cursorShape: Qt.PointingHandCursor
            anchors.fill: parent
            onClicked: {
                logsPopup.cancel();
            }
        }
    }
} // End of Item
