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
    id: dnsPopup
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.verticalCenter: parent.verticalCenter

    property string title
    property string subtitle
    property string dns1
    property string dns2

    signal cancel()
    signal dnsIPsSaved(string dns1, string dns2)

    ShadowRect {
        width: 335
        // Add 36 for top margin and 36 for bottom margin
        height: inputColumn.childrenRect.height + 24 + 24
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
                text: dnsPopup.title
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Item {
                width: parent.width
                height: 8
            }

            Text {
                font.family: "Roboto"
                font.bold: true
                font.pixelSize: 16
                width: 295
                wrapMode: Text.WordWrap
                color: "#6C798F"
                text: dnsPopup.subtitle
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Item {
                height: 24
                width: parent.width
            }

            HintedTextField {
                id: dns1Box
                width: 295
                height: 56
                hint: qsTr("PRIMARY");
                value: dnsPopup.dns1
            }

            Item {
                height: 24
                width: parent.width
            }

            HintedTextField {
                id: dns2Box
                width: 295
                height: 56
                hint: qsTr("SECONDARY");
                value: dnsPopup.dns2
            }

            Item {
                height: 16
                width: parent.width
            }

            Row {
                id: buttonsRow
                spacing: 0

                ShadowRect {
                    id: saveButton
                    width: 140
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
                            dnsPopup.dnsIPsSaved(dns1Box.value, dns2Box.value);
                        }
                    }
                }

                Item {
                    width: 15
                    height: parent.height
                }

                ShadowRect {
                    id: cancelButton
                    width: 140
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
                        onClicked: { dnsPopup.cancel(); }
                    }
                }
            }
        } // End of Column
    } // End of ShadowRect
} // End of Column
