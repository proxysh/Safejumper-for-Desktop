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

import vpn.server 1.0

Rectangle {
    id: serverCard

    property bool expandable: true

    // Whether or not to show encryption and port options
    property bool showOptions: false

    // Whether or not to show connect/disconnect button
    property bool showButton: false

    property Server currentServer
    property string isocode: currentServer.iso
    property string flagImage: "../roundflags/" + currentServer.iso+ ".png"
    property string serverEncryptionType: "SHA1"
    property string serverPort: "500"

    signal showLogin();

    function toggleExpansion() {
        // Do nothing if not expandable
        if (expandable) {
            if (showOptions) {
                showOptions = false;
            } else {
                showOptions = true;
            }

            serverCard.height = serverColumn.childrenRect.height + 20
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
                Layout.preferredWidth: 37
                Layout.preferredHeight: 37
                fillMode: Image.Stretch
                source: serverCard.flagImage
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                Image {
                    anchors.centerIn: flagImage
                    width: 42
                    height: 42
                    fillMode: Image.Stretch
                    source: "../images/green-ring.png"
                }
            }

            Column {
                spacing: 0
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                RowLayout {
                    width: parent.width
                    height: 16

                    Image {
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        width: 8
                        height: 8
                        source: vpnservicemanager.stateDot
                    }

                    Text {
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        id: statusText
                        text: vpnservicemanager.stateWord;
                        font.family: "Roboto-Bold"
                        font.bold: true
                        font.pixelSize: 12
                        color: vpnservicemanager.stateColor
                    }
                }

                Text {
                    id: nameText
                    text: currentServer.name
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
                source: currentServer.favorite ? "../images/filledheart.png" : "../images/emptyheart.png";

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    onClicked: {
                        toggleFavorite();
                    }
                }
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
                text: currentServer.load + "%"
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
                text: (currentServer.ping > 0 ? currentServer.ping + " ms" : "");
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
            color: vpnservicemanager.vpnState == 0 ? "#2CC532" : vpnservicemanager.vpnState == 1 ? "#FFAB00" : "#C53232"
            visible: showButton

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                text: vpnservicemanager.vpnState == 0 ? qsTr("CONNECT") : vpnservicemanager.vpnState == 1 ? qsTr("CONNECTING") : qsTr("DISCONNECT");
                color: 'white'
                font.family: "Roboto-Bold"
                font.bold: true
                font.pixelSize: 16
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    screen.connectToVPN();
                }
            }
        }

    }
} // End of rect
