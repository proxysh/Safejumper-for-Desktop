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

Rectangle {
    id: serverCard

    property bool expandable: true

    // Whether or not to show encryption and port options
    property bool showOptions: false

    // Whether or not to show connect/disconnect button
    property bool showButton: false

    property string serverName: "Hong Kong"
    property string isocode: "HK"
    property string flagImage: "../flags/" + isocode + ".png"
    property string serverLoad: "20%"
    property string serverPing: "5 ms"
    property string serverEncryptionType: "SHA1"
    property string serverPort: "500"

    function toggleExpansion() {
        // Do nothing if not expandable
        if (expandable) {
            if (showOptions)
                showOptions = false
            else
                showOptions = true
        }
    }

    width: 335
    height: serverColumn.childrenRect.height + 20 // 20px top margin
    color: "white"
    radius: 5

    layer.enabled: true
    layer.effect: DropShadow {
        horizontalOffset: 1
        verticalOffset: 1
        color: "#80000000"
    }

    Column {
        id: serverColumn
        anchors.topMargin: 20
        anchors.top: parent.top
        width: parent.width
        anchors.bottom: parent.bottom
        spacing: 0

        RowLayout {
            spacing: 16
            width: parent.width - 40
            height: 46
            anchors.horizontalCenter: parent.horizontalCenter // 20px margin on both sides

            Image {
                id: flagImage
                width: 35
                height: 35
                source: serverCard.flagImage
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            }

            Column {
                spacing: 0

                Text {
                    id: statusText
                    text: qsTr("NOT CONNECTED");
                    font.family: "Roboto-Bold"
                    font.bold: true
                    font.pixelSize: 12
                    color: "#C53232"
                }

                Text {
                    id: nameText
                    text: serverCard.serverName
                    font.family: "Roboto-Bold"
                    font.bold: true
                    font.pixelSize: 16
                    color: "#091E42"
                }
            }

            Image {
                Layout.alignment: Qt.AlignRight
                width: 20
                height: 18
                source: "../images/filledheart.png"
            }
        }

        Rectangle {
            id: divider1
            color: "#F4F5F7"
            width: parent.width;
            height: 1
        }

        RowLayout {
            // Ping and load row
            height: 44
            width: parent.width - 40 // 20px margin on both sides
            anchors.horizontalCenter: parent.horizontalCenter

            Image {
                id: loadIcon
                source: "../images/dashboard.png"
                width: 20
                height: 21
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            }

            Text {
                text: serverLoad
                font.family: "Roboto-Bold"
                font.pixelSize: 16
                font.bold: true
                color: "#7A869A"
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            }

            Rectangle {
                width: 100
                height: parent.height
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            }

            Image {
                id: pingIcon
                source: "../images/ping.png"
                width: 20
                height: 21
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }

            Text {
                text: serverPing
                font.family: "Roboto-Bold"
                font.pixelSize: 16
                font.bold: true
                color: "#7A869A"
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }

        }

        Rectangle {
            id: divider2
            color: "#F4F5F7"
            width: parent.width;
            height: 1
            visible: showOptions
        }

        RowLayout {
            id: encryptionTypeRow
            visible: showOptions

            height: 58
            width: parent.width - 40 // 20 px margin on both sides
            anchors.horizontalCenter: parent.horizontalCenter

            Column {
                spacing: 0
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                Text {
                    text: qsTr("ENCRYPTION TYPE");
                    font.family: "Roboto-Bold"
                    font.bold: true
                    font.pixelSize: 12
                    color: "#6C798F"
                }

                Text {
                    text: serverEncryptionType
                    font.family: "Roboto-Medium"
                    font.pixelSize: 16
                    color: "#172B4D"
                }
            }
        }

        Rectangle {
            id: divider3
            color: "#F4F5F7"
            width: parent.width;
            height: 1
            visible: showOptions
        }

        RowLayout {
            id: portRow
            visible: showOptions

            height: 58
            width: parent.width - 40 // 20 px margin on both sides
            anchors.horizontalCenter: parent.horizontalCenter

            Column {
                spacing: 0
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                Text {
                    text: qsTr("PORT NO");
                    font.family: "Roboto-Bold"
                    font.bold: true
                    font.pixelSize: 12
                    color: "#6C798F"
                }

                Text {
                    text: serverPort
                    font.family: "Roboto-Medium"
                    font.pixelSize: 16
                    color: "#172B4D"
                }
            }
        }

        Rectangle {
            id: connectButton
            width: parent.width
            height: 48
            radius: 5
            color: "#2CC532"
            visible: showButton

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("CONNECT");
                color: 'white'
                font.family: "Roboto-Bold"
                font.bold: true
                font.pixelSize: 16
            }

            MouseArea {
                anchors.fill: parent
                onClicked: { mainwindow.connect(); }
            }
        }

    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: { toggleExpansion(); }
    }
} // End of rect
