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

Item {
    id: signupPage

    signal closeClicked()
    signal passwordsDontMatch()

    property bool blur: false

    function showPopup(component)
    {
        signupPage.blur = true;
        component.visible = true;
    }

    function hidePopup(component)
    {
        signupPage.blur = false;
        component.visible = false;
    }

    FastBlur {
        id: blurItem
        source: signupRect
        radius: 32
        anchors.fill: parent
        z: 10
        visible: signupPage.blur
    }

    ErrorPopup {
        id: passwordsPopup
        visible: false
        title: qsTr("Error");
        subtitle: qsTr("Passwords do not match.");
        z: 20

        onCancel: { hidePopup(passwordsPopup); }
    }

    Rectangle {
        id: signupRect
        color: "#F4F5F7"
        anchors.fill: parent

        Column {
            id: signupColumn
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
                    onClicked: { signupPage.closeClicked() }
                }
            }

            Item {
                height: 23
                width: parent.width
            }

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                source: "../images/large-logo.png"
                width: 80
                height: 80
            }

            Item {
                height: 36
                width: parent.width
            }

            Text {
                font.family: "Roboto"
                font.bold: true
                color: "#172B4D"
                font.pixelSize: 24
                text: qsTr("Get started absolutely free.");
            }

            Item {
                height: 28
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

                        background: Rectangle {
                                  color: "transparent"
                                  border.color: "transparent"
                              }
                    }
                }
            }

            // Full name box
            ShadowRect {
                id: nameBox
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
                        text: qsTr("FULL NAME");
                    }

                    TextField {
                        id: nameInput
                        width: parent.width
                        leftPadding: 0
                        placeholderText: qsTr("Full name");
                        font.pixelSize: 16
                        color: "#97A0AF"
                        font.family: "Roboto"
                        font.bold: true

                        background: Rectangle {
                                  color: "transparent"
                                  border.color: "transparent"
                              }
                    }
                }
            }

            // Password box
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

            ShadowRect {
                id: passwordBox2
                width: parent.width
                height: 56

                color: "white"
                radius: 5

                TextField {
                    id: passwordInput2
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

                    placeholderText: qsTr("Re-Enter Password");

                    background: Rectangle {
                              color: "transparent"
                              border.color: "transparent"
                          }

                    Image {
                        anchors.right: passwordInput2.right
                        anchors.verticalCenter: passwordInput2.verticalCenter
    //                    anchors.rightMargin: 16
                        source: "../images/eye.png"

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (passwordInput2.echoMode == TextInput.Password)
                                    passwordInput2.echoMode = TextInput.Normal
                                else
                                    passwordInput2.echoMode = TextInput.Password
                            }
                        }

                    }
                }
            }

            Item {
                height: 16
                width: parent.width
            }

            Text {
                width: 300
                horizontalAlignment: Text.AlignHCenter
                font.family: "Roboto"
                font.pixelSize: 16
                wrapMode: Text.WordWrap

                text: qsTr("By Signing Up I agree to the Terms of Service and Privacy Policy");
            }

            Item {
                height: 24
                width: parent.width
            }

            ShadowRect {
                id: createAccountButton
                width: parent.width
                height: 56
                color: defaultColor
                radius: 5

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 16
                    font.family: "Roboto"
                    font.bold: true
                    color: "white"
                    text: qsTr("CREATE AN ACCOUNT");
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (passwordInput.text == passwordInput2.text) {
                            authmanager.createAccount(emailInput.text, nameInput.text, passwordInput.text);
                        } else {
                            showPopup(passwordsPopup)
                        }
                    }
                }
            }
        }
    }
}
