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
    id: updatesPopup
    anchors.bottom: parent.bottom
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.bottomMargin: 20
    spacing: 8

    property string subtitle

    property bool updateAvailable: false

    signal cancel()

    function checkForUpdates()
    {
        // Show checking for updates text
        subtitle = qsTr("Checking for updates...");
        updateAvailable = false;
        authmanager.checkForUpdates();
    }

    Connections {
        target: authmanager
        onNewVersionFound: {
            subtitle = qsTr("New version found, update?");
            updateAvailable = true;
        }
        onNoUpdateFound: {
            subtitle = qsTr("No update found.");
            updateAvailable = false;
        }
    }

    ShadowRect {
        width: 335
        // Add 20 for top margin and 20 for bottom margin
        height: inputColumn.childrenRect.height + 20 + 20
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
                text: qsTr("Updates")
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Item {
                width: parent.width
                height: 12
            }

            Text {
                font.family: "Roboto"
                font.bold: true
                font.pixelSize: 16
                width: 295
                wrapMode: Text.WordWrap
                color: "#6C798F"
                text: updatesPopup.subtitle
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    ShadowRect {
        id: downloadButton
        width: 335
        height: 48
        color: "#97A0AF"
        radius: 5

        visible: updateAvailable

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            font.family: "Roboto"
            font.bold: true
            font.pixelSize: 16
            text: qsTr("Download");
            color: "white"

            MouseArea {
                cursorShape: Qt.PointingHandCursor
                anchors.fill: parent
                onClicked: {
                    updatesPopup.cancel();
                    mainwindow.launchUrl(updateUrl);
                }
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
            text: qsTr("Cancel");
            color: "white"
        }

        MouseArea {
            cursorShape: Qt.PointingHandCursor
            anchors.fill: parent
            onClicked: { updatesPopup.cancel(); }
        }
    }

} // End of Item
