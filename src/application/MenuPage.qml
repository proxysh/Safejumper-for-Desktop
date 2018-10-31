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
import QtGraphicalEffects 1.0

Rectangle {
    id: menuPage
    color: defaultColor

    signal closeClicked();
    signal settingsClicked();
    signal logoutClicked();

    property bool blur: false

    function showPopup(component)
    {
        blur = true;
        component.visible = true;
    }

    function hidePopup(component)
    {
        blur = false;
        component.visible = false;
    }

    FastBlur {
        id: blurItem
        source: menuColumn
        radius: 32
        anchors.fill: parent
        z: 10
        visible: blur
    }

    LogsPopup {
        id: logsPopup
        visible: false
        z: 20

        onCopy: {
            // Copy logs content to clipboard
            hidePopup(logsPopup);
            mainwindow.copyLogsToClipboard();
        }

        onCancel: {
            hidePopup(logsPopup);
        }

    }

    Column {
        id: menuColumn
        anchors.fill: parent

        Item {
            id: headerArea
            width: parent.width
            height: 64

            Image {
                id: closeButton
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: 30
                source: "../images/x-circle.png"

                MouseArea {
                    anchors.fill: parent
                    onClicked: { menuPage.closeClicked(); }
                }
            }
        }

        Image {
            id: divider1
            anchors.topMargin: 15
            anchors.bottomMargin: 16
            anchors.horizontalCenter: parent.horizontalCenter
            source: "../images/divider.png"
        }

        MenuButton {
            id: mapButton
            buttonIcon: "../images/map.png"
            hoverButtonIcon: "../images/map-hover.png"
            buttonText: qsTr("Map View");
            highlighted: true
            onClicked: { menuPage.closeClicked(); }
        }

        MenuButton {
            id: serverButton
            buttonIcon: "../images/server.png"
            hoverButtonIcon: "../images/server-hover.png"
            buttonText: qsTr("Server List");
        }

//        MenuButton {
//            id: notificationsButton
//            buttonIcon: "../images/notification.png"
//            hoverButtonIcon: "../images/notification-hover.png"
//            buttonText: qsTr("Notifications");
//        }

        MenuButton {
            id: logsButton
            buttonIcon: "../images/list.png"
            hoverButtonIcon: "../images/list-hover.png"
            buttonText: qsTr("Show Logs");
            onClicked: {
                showPopup(logsPopup);
            }
        }

        MenuButton {
            id: shopButton
            buttonIcon: "../images/shopping.png"
            hoverButtonIcon: "../images/shopping-hover.png"
            buttonText: qsTr("Shop");
            onClicked: { mainwindow.launchUrl(shopUrl); }
        }

        MenuButton {
            id: settingsButton
            buttonIcon: "../images/settings.png"
            hoverButtonIcon: "../images/settings-hover.png"
            buttonText: qsTr("Settings");
            onClicked: { menuPage.settingsClicked(); }
        }

        MenuButton {
            id: logoutButton
            buttonIcon: "../images/logout.png"
            hoverButtonIcon: "../images/logout-hover.png"
            buttonText: qsTr("Logout");
            onClicked: { menuPage.logoutClicked(); }
        }

        Image {
            id: divider2
            anchors.topMargin: 15
            anchors.bottomMargin: 16
            anchors.horizontalCenter: parent.horizontalCenter
            source: "../images/divider.png"
        }

        // Now for the user's details
        Rectangle {
            id: userDetailsCard
            width: parent.width
            height: 200
            color: defaultColor
            opacity: authmanager.loggedIn ? 1 : 0

            Row {
                anchors.fill: parent
                anchors.topMargin: 24
                anchors.leftMargin: 48
                spacing: 16
                Image {
                    source: "../images/person-white.png"
                }

                Column {
                    spacing: 16

                    Text {
                        font.pixelSize: 18
                        font.family: "Roboto"
                        font.bold: true
                        color: "white"
                        text: authmanager.email
                    }

                    Text {
                        font.pixelSize: 18
                        font.family: "Roboto"
                        font.bold: true
                        color: "white"
                        text: authmanager.expiration
                    }

                    Text {
                        font.pixelSize: 18
                        font.family: "Roboto"
                        font.bold: true
                        color: "white"
                        text: authmanager.subscription
                    }
                }
            }
        }

    } // End of column
} // End of rect
