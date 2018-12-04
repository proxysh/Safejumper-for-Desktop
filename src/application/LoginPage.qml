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
    id: loginPage
    color: "#F4F5F7"

    signal closeClicked()
//    signal signupClicked()

    function loginFinished()
    {
        loginButton.enabled = true;
    }

    function performLogin()
    {
        loginButton.enabled = false;
        authmanager.login(emailBox.value, passwordBox.value);
    }

    Column {
        id: loginColumn
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.bottom: parent.bottom
        width: parent.width - 48 // 24 px margin on both sides
        spacing: 10

        Image {
            id: backImage
            anchors.left: parent.left
            source: "../images/back-black.png"

            MouseArea {
                cursorShape: Qt.PointingHandCursor
                anchors.fill: parent
                onClicked: { loginPage.closeClicked() }
            }
        }

        Item {
            height: 24
            width: parent.width
        }

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            source: "../images/large-logo.png"
            width: 128
            height: 128
        }

        Item {
            height: 48
            width: parent.width
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            height: 28
            font.family: "Roboto"
            font.bold: true
            font.pixelSize: 24
            color: "#172B4D"
            text: qsTr("Log into your account.");
        }

        Item {
            height: 24
            width: parent.width
        }

        HintedTextField {
            id: emailBox
            width: 327
            height: 56
            hint: qsTr("EMAIL");
            value: settings.rememberMe ? settings.login : ""
            onAccepted: { performLogin(); }
        }

        // Now logout button/box
        HintedTextField {
            id: passwordBox
            password: true
            width: parent.width
            height: 56
            hint: qsTr("PASSWORD");
            value: settings.rememberMe ? settings.password : ""
            onAccepted: { performLogin(); }
        }

        CheckBox {
            id: rememberButton
            checked: settings.rememberMe
            text: qsTr("Remember me");
            font.family: "Roboto"
            font.weight: Font.Medium
            font.pixelSize: 16

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                acceptedButtons: Qt.NoButton
            }

            indicator: Image {
                x: rememberButton.leftPadding
                y: parent.height / 2 - height / 2
                width: 26
                height: 26
                source: rememberButton.checked ? "../images/checkedcircle.png" : "../images/emptycircle.png";
            }

            contentItem: Text {
                      text: rememberButton.text
                      font: rememberButton.font
                      color: "#091E42"
                      verticalAlignment: Text.AlignVCenter
                      leftPadding: rememberButton.indicator.width + rememberButton.spacing
                  }

            onCheckStateChanged: {
                settings.rememberMe = rememberButton.checked;
                if (!settings.rememberMe) {
                    emailBox.value = "";
                    passwordBox.value = "";
                }
            }
        }

        Button {
            id: loginButton
            width: parent.width
            height: 56

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                acceptedButtons: Qt.NoButton
            }

            background: ShadowRect {
                opacity: enabled ? 1 : 0.3
                color: defaultColor
                radius: 5
            }

            contentItem: Text {
                opacity: enabled ? 1.0 : 0.3
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 16
                font.family: "Roboto"
                font.bold: true
                color: "white"
                text: qsTr("LOGIN");
            }

            onClicked: {
                performLogin();
            }
        }

        Item {
            // spacer
            width: parent.width
            height: 48
        }

//        Text {
//            text: qsTr("Don't have an account?");
//            color: "#020433"
//            anchors.horizontalCenter: parent.horizontalCenter
//            font.family: "Roboto"
//            font.weight: Font.Medium
//            font.pixelSize: 16
//        }

//        ShadowRect {
//            id: signupButton
//            color: "white"
//            width: parent.width
//            height: 56
//            radius: 5

//            Text {
//                anchors.verticalCenter: parent.verticalCenter
//                anchors.horizontalCenter: parent.horizontalCenter
//                font.family: "Roboto"
//                font.bold: true
//                font.pixelSize: 16
//                color: defaultColor
//                text: qsTr("SIGN UP");
//            }

//            MouseArea {
//                cursorShape: Qt.PointingHandCursor
//                anchors.fill: parent

//                onClicked: { loginPage.signupClicked(); }
//            }
//        }
    }
}
