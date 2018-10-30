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
    id: headerArea
    width: parent.width
    height: 57
    anchors.top: parent.top

    signal menuClicked()
    signal settingsClicked()

    Rectangle {
        id: toolBarTop
        anchors.fill: parent
        color: defaultColor

        layer.enabled: true
        layer.effect: DropShadow {
            horizontalOffset: 1
            verticalOffset: 1
            color: "#80000000"
        }

        Image {
            id: menuImage
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 20
            source: "../images/menu-left.png"

            MouseArea {
                anchors.fill: parent
                onClicked: { headerArea.menuClicked() }
            }
        }

        Row {
            id: centerBox
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 8

            Image {
                id: statusIcon
                source: vpnservicemanager.stateIcon
            }

            Text {
                id: statusText
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: 18
                font.family: "Roboto"
                font.bold: true
                color: "white"
                text: vpnservicemanager.stateWord
            }
        }

        Image {
            id: settingsImage
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 20
            source: "../images/settings.png"

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    headerArea.settingsClicked();
                }
            }
        }
    }
}
