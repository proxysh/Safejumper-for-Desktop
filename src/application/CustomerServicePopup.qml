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

ShadowRect {
    id: customerServicePopup
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.verticalCenter: parent.verticalCenter
    width: 335
    height: messageColumn.childrenRect.height
    color: "white"
    radius: 5

    signal launchCSSite()

    ColumnLayout {
        id: messageColumn
        width: 335
        anchors.fill: parent
        spacing: 0

        Item {
            // Space above "Customer Service" label
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: 72
        }

        Text {
            font.family: "Roboto"
            font.bold: true
            font.pixelSize: 20
            lineHeight: 24
            lineHeightMode: Text.FixedHeight
            color: "#172B4D"
            text: qsTr("Customer Service");
            Layout.alignment: Qt.AlignHCenter
        }

        Item {
            // Space between "Customer Service" label and image
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: 36
        }

        Image {
            source: "../images/customerservice.png"
            Layout.preferredWidth: 239
            Layout.preferredHeight: 236
            Layout.alignment: Qt.AlignHCenter
        }

        Item {
            // Space below image
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: 36
        }

        Text {
            width: 295
            font.family: "Roboto"
            font.pixelSize: 16
            font.weight: Font.Normal
            font.bold: false
            lineHeight: 24
            lineHeightMode: Text.FixedHeight
            color: "#505F79"
            text: qsTr("Are you stuck with something?<br />Let's get in touch. We will help you troubleshoot your problems.");
            wrapMode: Text.WordWrap
            horizontalAlignment: Qt.AlignHCenter
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 295
        }

        Item {
            // Space below text
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: 22
        }

        ShadowRect {
            id: contactButton
            width: 295
            height: 48
            color: "#2CC532"
            radius: 5
            Layout.alignment: Qt.AlignHCenter

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                font.family: "Roboto"
                font.bold: true
                font.pixelSize: 16
                text: qsTr("CONTACT US NOW");
                color: "white"
            }

            MouseArea {
                cursorShape: Qt.PointingHandCursor
                anchors.fill: parent
                onClicked: {
                    customerServicePopup.launchCSSite();
                }
            }
        }

        Item {
            // Space below text
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: 48
        }
    }
}
