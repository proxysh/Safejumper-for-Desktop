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
    }

    Component {
        id: mapPage
        MapPage {
            onMenuClicked: { stack.push(menuPage); }
            onSettingsClicked: { stack.push(settingsPage); }
        }
    }

    Component {
        id: menuPage
        MenuPage {
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
            onCloseClicked: { stack.pop(); }
            onSignupClicked: { stack.push(signupPage); }
        }
    }

    Component {
        id: signupPage
        SignupPage {
            onCloseClicked: { stack.pop(); }

        }
    }

    StackView {
        id: stack
        initialItem: mapPage
        anchors.fill: screen
    }
}
