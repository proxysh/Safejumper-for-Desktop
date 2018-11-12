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

ShadowRect {
    id: serverCard

    property bool expandable: true

    // Whether or not to show encryption and port options
    property bool showOptions: false

    // Whether or not to show connect/disconnect button
    property bool showButton: false

    // Whether to show connected/not connected above server name
    property bool showState: false

    // True if this server is currently selected
    property bool isCurrentServer: false

    property Server currentServer
    property string isocode: currentServer.iso
    property string flagImage: "../roundflags/" + currentServer.iso+ ".png"
    property string serverEncryptionType: "SHA1"
    property string serverPort: "500"

    signal showLogin();

    function toggleFavorite() {
        currentServer.favorite = !currentServer.favorite;
    }

    function toggleExpansion() {
        // Do nothing if not expandable
        if (expandable) {
            if (showOptions) {
                showOptions = false;
                showButton = false;
            } else {
                showOptions = true;
                showButton = true;
            }
        }
    }

    width: 335
    height: serverColumn.height
    color: "white"
    radius: 5

    MouseArea {
        cursorShape: expandable ? Qt.PointingHandCursor : Qt.ArrowCursor
        anchors.fill: parent
        onClicked: {
            toggleExpansion();
        }
    }

    Column {
        id: serverColumn
        width: parent.width
        anchors.bottom: parent.bottom
        spacing: 0
        padding: 0

        RowLayout {
            spacing: 16
            width: parent.width - 40
            height: 66
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
                    source: (isCurrentServer & vpnservicemanager.vpnState == 2)? "../images/green-ring.png" : "../images/grey-ring.png"
                }
            }

            ColumnLayout {
                spacing: 0
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                RowLayout {
                    visible: showState
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
                        font.family: "Roboto"
                        font.bold: true
                        font.pixelSize: 12
                        color: vpnservicemanager.stateColor
                    }
                }

                Text {
                    id: nameText
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    text: currentServer.name
                    font.family: "Roboto"
                    font.bold: true
                    font.pixelSize: 16
                    color: "#091E42"
                    horizontalAlignment: Text.AlignLeft
                }
            }

            Item {
                Layout.fillWidth: true;
                height: parent.height
            }

            Image {
                Layout.alignment: Qt.AlignRight
                width: 20
                height: 18
                source: currentServer.favorite ? "../images/filledheart.png" : "../images/emptyheart.png";

                MouseArea {
                    cursorShape: Qt.PointingHandCursor
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
            height: 45
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
                font.family: "Roboto"
                font.pixelSize: 16
                font.bold: true
                color: "#7A869A"
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            }

            Rectangle {
                width: 1
                height: parent.height
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                Layout.fillWidth: true
            }

            Image {
                id: pingIcon
                source: "../images/ping.png"
                visible: settings.pingEnabled
                width: 20
                height: 21
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }

            Text {
                visible: settings.pingEnabled
                text: (currentServer.ping > 0 ? currentServer.ping + " ms" : "");
                font.family: "Roboto"
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

        ComboBox {
            id: encryptionBox
            width: parent.width
            visible: showOptions
            font.family: "Roboto"
            font.weight: Font.Medium
            font.pixelSize: 16
            currentIndex: settings.serverEncryption(currentServer.address)

            onActivated: {
                settings.setServerEncryption(currentServer.address, index);
            }

            model: encryptionModel

            delegate: ItemDelegate {
                      width: encryptionBox.width
                      contentItem: Text {
                          text: modelData
                          font: encryptionBox.font
                          elide: Text.ElideRight
                          verticalAlignment: Text.AlignVCenter
                      }
                highlighted: encryptionBox.highlightedIndex === index
            }

            indicator: Image {
                x: encryptionBox.width - width - encryptionBox.rightPadding
                y: encryptionBox.topPadding + (encryptionBox.availableHeight - height) / 2
                width: 10
                height: 6
                source: "../images/down-arrow.png"
            }

            contentItem: Column {
                topPadding: 12
                leftPadding: 20
                rightPadding: encryptionBox.indicator.width + encryptionBox.spacing
                spacing: 2

                Text {
                    font.family: "Roboto"
                    font.weight: Font.Black
                    font.pixelSize: 12
                    color: "#6C798F"
                    text: qsTr("ENCRYPTION TYPE");
                }

                Text {
                     text: encryptionBox.displayText
                     font: encryptionBox.font
                     color: "#172B4D"
                     verticalAlignment: Text.AlignVCenter
                     elide: Text.ElideRight
                 }
            }

             background: Rectangle {
                 implicitWidth: 120
                 implicitHeight: 58
                 color: "white"
             }
        }

        Rectangle {
            id: divider3
            color: "#F4F5F7"
            width: parent.width;
            height: 1
            visible: showOptions
        }

        ComboBox {
            id: portBox
            width: parent.width
            visible: showOptions
            font.family: "Roboto"
            font.weight: Font.Medium
            font.pixelSize: 16
            currentIndex: settings.serverProtocol(currentServer.address, encryptionBox.currentIndex)

            onActivated: {
                settings.setServerProtocol(currentServer.address, encryptionBox.currentIndex, index);
            }

            model: settings.portsForEncryption(encryptionBox.currentIndex)

            delegate: ItemDelegate {
                      width: portBox.width
                      contentItem: Text {
                          text: modelData
                          font: portBox.font
                          elide: Text.ElideRight
                          verticalAlignment: Text.AlignVCenter
                      }
                highlighted: portBox.highlightedIndex === index
            }

            indicator: Image {
                x: portBox.width - width - portBox.rightPadding
                y: portBox.topPadding + (portBox.availableHeight - height) / 2
                width: 10
                height: 6
                source: "../images/down-arrow.png"
            }

            contentItem: Column {
                topPadding: 12
                leftPadding: 20
                rightPadding: portBox.indicator.width + portBox.spacing
                spacing: 2

                Text {
                    font.family: "Roboto"
                    font.weight: Font.Black
                    font.pixelSize: 12
                    color: "#6C798F"
                    text: qsTr("PORT NO");
                }

                Text {
                     text: portBox.displayText
                     font: portBox.font
                     color: "#172B4D"
                     verticalAlignment: Text.AlignVCenter
                     elide: Text.ElideRight
                 }
            }

             background: Rectangle {
                 implicitWidth: 120
                 implicitHeight: 58
                 color: "white"
             }
        }

        Connections {
            target: vpnservicemanager
            onVpnStateChanged: {
                connectButton.enabled = true;
            }
        }

        Button {
            id: connectButton
            width: parent.width
            height: 49
            visible: showButton

            background: Rectangle {
                radius: 5
                color: vpnservicemanager.vpnState == 0 ? "#2CC532" : vpnservicemanager.vpnState == 1 ? "#FFAB00" : "#C53232"
                opacity: enabled ? 1.0 : 0.3
                layer.enabled: true

                Rectangle {
                    id: removeTopCorners
                    width: parent.width
                    anchors.top: parent.top
                    height: 5
                    color: parent.color
                }

            }

            contentItem: Text {
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                text: vpnservicemanager.vpnState == 0 ? qsTr("CONNECT") : vpnservicemanager.vpnState == 1 ? qsTr("CONNECTING") : qsTr("DISCONNECT");
                color: 'white'
                opacity: enabled ? 1.0 : 0.3
                font.family: "Roboto"
                font.bold: true
                font.pixelSize: 16
            }

            onClicked: {
                connectButton.enabled = false

                // Select this server if it's not the current server
                if (settings.server != currentServer.id)
                    settings.server = currentServer.id;

                screen.connectToVPN();
            }
        }

    }
} // End of rect
