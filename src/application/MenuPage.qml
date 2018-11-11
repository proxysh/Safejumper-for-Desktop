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
    signal mapClicked();
    signal allServersClicked();
    signal settingsClicked();
    signal logoutClicked();
    signal logsClicked();

    function clearHighlights()
    {
        mapButton.highlighted = false;
        serverButton.highlighted = false;
        logsButton.highlighted = false;
        shopButton.highlighted = false;
        settingsButton.highlighted = false;
        logoutButton.highlighted = false;
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
                    cursorShape: Qt.PointingHandCursor
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
            onClicked: {
                clearHighlights();
                mapButton.highlighted = true;
                menuPage.mapClicked();
            }
        }

        MenuButton {
            id: serverButton
            buttonIcon: "../images/server.png"
            hoverButtonIcon: "../images/server-hover.png"
            buttonText: qsTr("Server List");
            onClicked: {
                clearHighlights();
                serverButton.highlighted = true;
                menuPage.allServersClicked();
            }
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
                clearHighlights();
                logsButton.highlighted = true;
                menuPage.logsClicked();
            }
        }

        MenuButton {
            id: shopButton
            buttonIcon: "../images/shopping.png"
            hoverButtonIcon: "../images/shopping-hover.png"
            buttonText: qsTr("Shop");
            onClicked: {
                clearHighlights();
                shopButton.highlighted = true;
                mainwindow.launchUrl(shopUrl);
            }
        }

        MenuButton {
            id: settingsButton
            buttonIcon: "../images/settings.png"
            hoverButtonIcon: "../images/settings-hover.png"
            buttonText: qsTr("Settings");
            onClicked: {
                clearHighlights();
                settingsButton.highlighted = true;
                menuPage.settingsClicked();
            }
        }

        MenuButton {
            id: logoutButton
            buttonIcon: "../images/logout.png"
            hoverButtonIcon: "../images/logout-hover.png"
            buttonText: authmanager.loggedIn ? qsTr("Logout") : qsTr("Login");
            onClicked: {
                clearHighlights();
                logoutButton.highlighted = true;
                menuPage.logoutClicked();
            }
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
