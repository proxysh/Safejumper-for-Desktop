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
    id: screen
    focus: true
    color: "white"
    width: 375
    height: 755

    property color defaultColor: "#FEBE10"
    property string shopUrl: "https://www.mbaex.com"

    property bool blur: false

    function showPopup(component)
    {
        screen.blur = true;
        component.visible = true;
    }

    function hidePopup(component)
    {
        screen.blur = false;
        component.visible = false;
    }

    FastBlur {
        id: blurItem
        source: stack
        radius: 32
        anchors.fill: parent
        z: 10
        visible: screen.blur
    }

    ConfirmationPopup {
        id: exitConfirmation
        title: qsTr("Exit");
        subtitle: qsTr("Would you like to shut Shieldtra down?");
        confirmText: qsTr("CONFIRM");
        cancelText: qsTr("CANCEL");
        visible: false
        z: 20

        onConfirm: {
            hidePopup(exitConfirmation);
            mainwindow.shutDown();
        }

        onCancel: {
            hidePopup(exitConfirmation);
        }

    }

    Connections {
        target: mainwindow
        onConfirmExit: { showPopup(exitConfirmation); }
        onLogout: {
            if (stack.currentItem.objectName == "loginPage")
                // Do nothing
                ;
            else if (stack.currentItem.objectName == "signupPage")
                stack.pop();
            else
                stack.push(loginPage);
        }
        onMapScreen: {
            if (stack.currentItem.objectName == "mapPage")
                // Do nothing
                ;
            else
                // Keep popping until we get back to the map page
                while (stack.currentItem.objectName != "mapPage")
                    stack.pop();
        }
        onSettingsScreen: {
            if (stack.currentItem.objectName == "settingsPage")
                // Do nothing
                ;
            else
                stack.push(settingsPage);
        }
        onLogsScreen: {
            // Show logs popup
        }
    }

    Connections {
        target: authmanager
        onServerListsLoaded: {
            splashScreen.visible = false;
        }
    }

    Component {
        id: allServersPage
        AllServersPage {
            objectName: "allServersPage"
        }
    }

    Component {
        id: mapPage
        MapPage {
            objectName: "mapPage"
            onMenuClicked: { stack.push(menuPage); }
            onSettingsClicked: { stack.push(settingsPage); }
            onAllServersClicked: { stack.push(allServersPage); }
        }
    }

    Component {
        id: menuPage
        MenuPage {
            objectName: "menuPage"
            onCloseClicked: { stack.pop(); }
            onSettingsClicked: { stack.push(settingsPage); }
            onLogoutClicked: {
                authmanager.logout();
                stack.push(loginPage);
            }
        }
    }

    Component {
        id: settingsPage
        SettingsPage {
            objectName: "settingsPage"
            onCloseClicked: { stack.pop(); }
            onLogoutClicked: {
                authmanager.logout();
                stack.push(loginPage);
            }
        }
    }

    Component {
        id: loginPage
        LoginPage {
            objectName: "loginPage"
            onCloseClicked: { stack.pop(); }
            onSignupClicked: { stack.push(signupPage); }
        }
    }

    Component {
        id: signupPage
        SignupPage {
            objectName: "signupPage"
            onCloseClicked: { stack.pop(); }

        }
    }

    Rectangle {
        id: splashScreen
        visible: true
        anchors.fill: screen
        z: 10

        Image {
            id: splashLogo
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 240
            width: 210
            height: 240
            source: "../images/large-logo.png"
        }

        Text {
            anchors.top: splashLogo.bottom
            anchors.topMargin: 24
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Establishing a secure connection...");
            font.family: "Roboto-Regular"
            font.pixelSize: 20
            color: defaultColor
        }
    }

    StackView {
        id: stack
        initialItem: mapPage
        anchors.fill: screen
    }
}
