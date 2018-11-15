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
    signal signupClicked()

    function loginFinished()
    {
        loginButton.enabled = true;
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

        ShadowRect {
            id: emailBox
            width: 327
            height: 56

            color: "white"
            radius: 5

            Column {
                anchors.fill: parent
                anchors.topMargin: 8
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 4

                Text {
                    font.pixelSize: 12
                    font.family: "Roboto"
                    font.weight: Font.Black
                    color: "#6C798F"
                    text: qsTr("EMAIL");
                }

                TextField {
                    id: emailInput
                    width: parent.width
                    leftPadding: 0
                    placeholderText: qsTr("Email address");
                    font.pixelSize: 16
                    color: "#97A0AF"
                    font.family: "Roboto"
                    font.bold: true

                    text: settings.rememberMe ? settings.login : ""

                    background: Rectangle {
                              color: "transparent"
                              border.color: "transparent"
                          }
                }
            }
        }

        // Now logout button/box
        ShadowRect {
            id: passwordBox
            width: parent.width
            height: 56

            color: "white"
            radius: 5

            TextField {
                id: passwordInput
                width: parent.width - 32
                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                echoMode: TextInput.Password
                leftPadding: 0
                font.pixelSize: 16
                color: "#97A0AF"
                font.family: "Roboto"
                font.bold: true

                placeholderText: qsTr("Password");

                text: settings.rememberMe ? settings.password : ""

                background: Rectangle {
                          color: "transparent"
                          border.color: "transparent"
                      }

                Image {
                    anchors.right: passwordInput.right
                    anchors.verticalCenter: passwordInput.verticalCenter
//                    anchors.rightMargin: 16
                    source: "../images/eye.png"

                    MouseArea {
                        cursorShape: Qt.PointingHandCursor
                        anchors.fill: parent
                        onClicked: {
                            if (passwordInput.echoMode == TextInput.Password)
                                passwordInput.echoMode = TextInput.Normal
                            else
                                passwordInput.echoMode = TextInput.Password
                        }
                    }

                }
            }
        }

        CheckBox {
            id: rememberButton
            checked: settings.rememberMe
            text: qsTr("Remember me");
            font.family: "Roboto"
            font.weight: Font.Medium
            font.pixelSize: 16

            indicator: Image {
                x: rememberButton.leftPadding
                y: parent.height / 2 - height / 2
                width: 26
                height: 26
                source: rememberButton.checked ? "../images/checkedcircle.png" : "../images/empty-radio-button.png";
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
            }
        }

        Button {
            id: loginButton
            width: parent.width
            height: 56
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
                enabled = false;
                authmanager.login(emailInput.text, passwordInput.text);
            }
        }

        Item {
            // spacer
            width: parent.width
            height: 60
        }

        Text {
            text: qsTr("Don't have an account?");
            color: "#020433"
            anchors.horizontalCenter: parent.horizontalCenter
            font.family: "Roboto"
            font.weight: Font.Medium
            font.pixelSize: 16
        }

        ShadowRect {
            id: signupButton
            color: "white"
            width: parent.width
            height: 56
            radius: 5

            Text {
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.family: "Roboto"
                font.bold: true
                font.pixelSize: 16
                color: defaultColor
                text: qsTr("SIGN UP");
            }

            MouseArea {
                cursorShape: Qt.PointingHandCursor
                anchors.fill: parent

                onClicked: { loginPage.signupClicked(); }
            }
        }
    }
}
