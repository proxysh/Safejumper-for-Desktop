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

    Rectangle {
        width: 335
        // Add 36 for top margin and 36 for bottom margin
        height: inputColumn.childrenRect.height + 36 + 36
        color: "white"
        radius: 5

        Column {
            id: inputColumn
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.topMargin: 36
            anchors.top: parent.top
            spacing: 0

            Text {
                font.family: "Roboto"
                font.bold: true
                font.pixelSize: 20
                color: defaultColor
                text: dnsPopup.title
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                font.family: "Roboto"
                font.bold: true
                font.pixelSize: 16
                width: 295
                wrapMode: Text.WordWrap
                color: "#6C798F"
                text: dnsPopup.subtitle
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Item {
                height: 24
                width: parent.width
            }

            Rectangle {
                width: 295
                height: 56

                color: "white"
                radius: 5

                layer.enabled: true
                layer.effect: DropShadow {
                    horizontalOffset: 1
                    verticalOffset: 1
                    color: "#80000000"
                }

                Column {
                    anchors.fill: parent
                    anchors.topMargin: 8
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 4

                    Text {
                        font.pixelSize: 12
                        font.family: "Roboto-Black"
                        color: "#6C798F"
                        text: qsTr("PRIMARY");
                    }

                    TextField {
                        id: dns1Box
                        width: parent.width
                        leftPadding: 0
                        placeholderText: qsTr("Enter value");
                        font.pixelSize: 16
                        color: "#97A0AF"
                        font.family: "Roboto"
                        font.bold: true

                        text: dnsPopup.dns1

                        background: Rectangle {
                                  color: "transparent"
                                  border.color: "transparent"
                              }
                    }
                }
            }

            Item {
                height: 24
                width: parent.width
            }

            Rectangle {
                width: 295
                height: 56

                color: "white"
                radius: 5

                layer.enabled: true
                layer.effect: DropShadow {
                    horizontalOffset: 1
                    verticalOffset: 1
                    color: "#80000000"
                }

                Column {
                    anchors.fill: parent
                    anchors.topMargin: 8
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 4

                    Text {
                        font.pixelSize: 12
                        font.family: "Roboto-Black"
                        color: "#6C798F"
                        text: qsTr("SECONDARY");
                    }

                    TextField {
                        id: dns2Box
                        width: parent.width
                        leftPadding: 0
                        placeholderText: qsTr("Enter value");
                        font.pixelSize: 16
                        color: "#97A0AF"
                        font.family: "Roboto"
                        font.bold: true

                        text: dnsPopup.dns2

                        background: Rectangle {
                                  color: "transparent"
                                  border.color: "transparent"
                              }
                    }
                }
            }

            Item {
                height: 16
                width: parent.width
            }

            Row {
                id: buttonsRow
                spacing: 0

                Rectangle {
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
                        anchors.fill: parent
                        onClicked: {
                            dnsPopup.dnsIPsSaved(dns1Box.text, dns2Box.text);
                        }
                    }

                    layer.enabled: true
                    layer.effect: DropShadow {
                        horizontalOffset: 1
                        verticalOffset: 1
                        color: "#80000000"
                    }
                }

                Item {
                    width: 15
                    height: parent.height
                }

                Rectangle {
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
                        anchors.fill: parent
                        onClicked: { dnsPopup.cancel(); }
                    }

                    layer.enabled: true
                    layer.effect: DropShadow {
                        horizontalOffset: 1
                        verticalOffset: 1
                        color: "#80000000"
                    }
                }
            }
        } // End of column

        layer.enabled: true
        layer.effect: DropShadow {
            horizontalOffset: 1
            verticalOffset: 1
            color: "#80000000"
        }
    } // End of rect

} // End of Item
