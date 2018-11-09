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
import QtGraphicalEffects 1.0

import vpn.server 1.0

Item {
    id: mapPage
//    anchors.fill: parent

    signal menuClicked()
    signal settingsClicked()
    signal allServersClicked()
    signal showLogin()

    property Server currentServer: serversModel.server(settings.server)

    function refresh()
    {
        currentServer = serversModel.server(settings.server)
        var iso = currentServer.iso
        currentServerCard.currentServer = currentServer;
        background.source = "../maps/" + iso + vpnservicemanager.stateMapSuffix
    }

    function stateChanged()
    {
        currentServer = serversModel.server(settings.server)
        var iso = currentServer.iso
        background.source = "../maps/" + iso + vpnservicemanager.stateMapSuffix
    }

    Connections {
        target: authmanager
        onServerListsLoaded: {
            refresh();
        }
    }

    Connections {
        target: settings
        onServerChanged: {
            refresh();
        }
    }

    Connections {
        target: vpnservicemanager
        onVpnStateChanged: {
            stateChanged();
        }
    }

    HeaderBar {
        onSettingsClicked: { mapPage.settingsClicked(); }
        onMenuClicked: { mapPage.menuClicked(); }
    }

    Image {
        id: background
        z: -1
        anchors.top: headerArea.top - 55
        anchors.left: parent.left
        source: "../maps/" + serversModel.server(settings.server).iso + vpnservicemanager.stateMapSuffix
    }

    Column {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 24
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        spacing: 12

        width: 335
        height: childrenRect.height

        RowLayout {
            width: parent.width

            Image {
                Layout.alignment: Qt.AlignVCenter | Qt.alignLeft
                source: "../images/chevron-left.png"

                MouseArea {
                    cursorShape: Qt.PointingHandCursor
                    anchors.fill: parent
                    onClicked: { authmanager.previousFavorite(); }
                }
            }

            ShadowRect {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                id: serverRectangle
                color: "white"
                radius: 5
                width: 260
                height: 48

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 35
                    anchors.rightMargin: 35

                    Image {
                        id: flagImage
                        Layout.preferredWidth: 26
                        Layout.preferredHeight: 26
                        fillMode: Image.Stretch
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        source: "../roundflags/" + currentServer.iso + ".png"
                        z: 2

                        Image {
                            anchors.centerIn: parent
                            width: 30
                            height: 30
                            fillMode: Image.Stretch
                            source: "../images/small-grey-ring.png"
                            z: 4
                        }
                    }


                    Text {
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        font.family: "Roboto-Medium"
                        font.pixelSize: 16
                        color: "#091E42"
                        text: currentServer.name
                    }

                    Item {
                        Layout.fillWidth: true
                        height: 1
                        width: 1
                    }
                }
            }

            Image {
                Layout.alignment: Qt.AlignVCenter | Qt.alignRight
                source: "../images/chevron-right.png"

                MouseArea {
                    cursorShape: Qt.PointingHandCursor
                    anchors.fill: parent
                    onClicked: { authmanager.nextFavorite(); }
                }
            }
        }

        ServerCard {
            id: currentServerCard
            showButton: true
            expandable: false
            showState: true
            isCurrentServer: true
        }

        ShadowRect {
            id: allServersButton
            width: parent.width
            height: 56
            color: "white"
            radius: 5

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("SHOW ALL SERVERS");
                color: defaultColor
                font.family: "Roboto"
                font.bold: true
                font.pixelSize: 16
            }

            MouseArea {
                cursorShape: Qt.PointingHandCursor
                anchors.fill: parent
                onClicked: {
                    mapPage.allServersClicked();
                }
            }
        }


    }
}
