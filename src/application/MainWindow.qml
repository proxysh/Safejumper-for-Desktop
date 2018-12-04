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
    scale: 0.8
    transformOrigin: Item.TopLeft

    property color defaultColor: "#FEBE10"
    property string shopUrl: "https://www.mbaex.com"
    property string updateUrl: "https://www.mbaex.com/software"

    property bool blur: false

    property var currentPopup

    ListModel {
        id: encryptionModel
        ListElement { name: "TLSCrypt" }
        ListElement { name: "TLSCrypt+XOR" }
    }

    function showPopup(component)
    {
        if (screen.blur) {
            // Already showing a popup, so hide all popups
            hidePopup(currentPopup);
        }

        screen.blur = true;
        component.visible = true;
        currentPopup = component;
    }

    function hidePopup(component)
    {
        screen.blur = false;
        component.visible = false;
    }

    function hidecurrentPopup()
    {
        hidePopup(currentPopup);
    }

    function showLogin()
    {
        if (stack.currentItem.objectName == "loginPage")
            // Do nothing
            ;
        else if (stack.currentItem.objectName == "signupPage")
            stack.pop();
        else
            stack.push(loginPage);
    }

    function connectToVPN()
    {
        if (!authmanager.loggedIn) {
            showLogin();
            return false;
        }

        // TODO: Confirm disconnect/jump if currently connected
        else if (vpnservicemanager.vpnState == 1 || vpnservicemanager.vpnState == 2) // Connecting
            vpnservicemanager.sendDisconnectFromVPNRequest();

        else
            vpnservicemanager.sendConnectToVPNRequest();

        return true;
    }

    function loginError(message)
    {
        errorPopup.title = qsTr("Incorrect credentials");
        errorPopup.subtitle = message;
        showPopup(errorPopup);
    }

    function logout()
    {
        if (authmanager.loggedIn)
            screen.showPopup(logoutConfirmation);
        else
            showLogin();
    }

    function showMap()
    {
        stack.pop(null);
    }

    function showAllServers()
    {
        stack.pop(null);
        stack.push(allServersPage);
    }

    FastBlur {
        id: blurItem
        source: stack
        radius: 32
        anchors.fill: parent
        z: 10
        visible: screen.blur

        MouseArea {
            anchors.fill: blurItem
            enabled: blurItem.visible
            onClicked: { hidecurrentPopup(); }
        }
    }

    ConfirmationPopup {
        id: logoutConfirmation
        title: qsTr("Logout");
        subtitle: qsTr("Would you like to log out?");
        confirmText: qsTr("CONFIRM");
        visible: false
        z: 20

        onConfirm: {
            screen.hidePopup(logoutConfirmation);
            authmanager.logout();
            stack.push(loginPage);
        }

        onCancel: {
            screen.hidePopup(logoutConfirmation);
        }
    }

    ConfirmationPopup {
        id: exitConfirmation
        title: qsTr("Exit");
        subtitle: qsTr("Would you like to shut the VPN software completely down?");
        confirmText: qsTr("CONFIRM");
        visible: false
        z: 20

        onConfirm: {
            screen.hidePopup(exitConfirmation);
            mainwindow.shutDown();
        }

        onCancel: {
            screen.hidePopup(exitConfirmation);
        }

    }

    CustomerServicePopup {
        id: customerServicePopup
        visible: false
        z: 20
        onLaunchCSSite: {
            screen.hidePopup(customerServicePopup);
            mainwindow.launchCustomerService();
        }
    }

    Connections {
        target: mainwindow
        onConfirmExit: {
            screen.showPopup(exitConfirmation);
        }
        onLogout: {
            logout();
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
            stack.pop(null); // First pop back to map page
            logsPopup.updateLogs();
            screen.showPopup(logsPopup);
        }
    }

    Connections {
        target: authmanager
        onServerListsLoaded: {
            splashScreen.visible = false;
        }
        onLoggedInChanged: {
            if (authmanager.loggedIn && stack.currentItem.objectName == "loginPage") {
                stack.pop();
                loginPage.loginFinished();
            }
        }
        onLoginError: {
            loginPage.loginFinished();
            loginError(message);
        }
    }

    // Popup used for selecting encryption for all servers cards
    SelectionPopup {
        id: encryptionPopup
        title: qsTr("Encryption");
        subtitle: qsTr("Select your desired encryption type.");
        visible: false
        itemModel: encryptionModel
        z: 20

        onItemSelected: {
            hidePopup(encryptionPopup);
            allServersPage.setEncryption(index);
        }

        onCancel: {
            hidePopup(encryptionPopup);
        }
    }

    // Popup used for selecting port on all servers page cards
    SelectionPopup {
        id: portPopup
        title: qsTr("Port");
        subtitle: qsTr("Select the port you would like to use.");
        visible: false
        itemModel: settings.ports
        z: 20

        onItemSelected: {
            hidePopup(portPopup);
            console.log("Item selected at index " + index);
            allServersPage.setPort(index);
        }

        onCancel: {
            hidePopup(portPopup);
        }
    }

    AllServersPage {
        id: allServersPage
        objectName: "allServersPage"
        onBackClicked: { stack.pop(); }
        onSettingsClicked: { stack.push(settingsPage); }
        onSelectEncryption: {
            console.log("onSelectEncryption called in mainwindow");
            encryptionPopup.selectedIndex = settings.serverEncryption(serversModel.server(index).address);
            showPopup(encryptionPopup);
        }

        onSelectPort: {
            var serverAddress = serversModel.server(index).address;
            var encryption = settings.serverEncryption(serverAddress)
            portPopup.selectedIndex = settings.serverProtocol(serverAddress, encryption);
            showPopup(portPopup);
        }
    }

    MapPage {
        id: mapPage
        objectName: "mapPage"
        onMenuClicked: { stack.push(menuPage); }
        onSettingsClicked: { stack.push(settingsPage); }
        onAllServersClicked: { stack.push(allServersPage); }
    }

    MenuPage {
        id: menuPage
        objectName: "menuPage"
        onCloseClicked: { stack.pop(); }
        onMapClicked: { showMap(); }
        onAllServersClicked: { showAllServers(); }
        onSettingsClicked: { stack.push(settingsPage); }
        onLogoutClicked: {
            screen.logout();
        }
        onLogsClicked: {
            stack.pop(null); // First pop back to map page
            logsPopup.updateLogs();
            screen.showPopup(logsPopup);
        }
    }

    SettingsPage {
        id: settingsPage
        objectName: "settingsPage"
        onCloseClicked: { stack.pop(); }
        onLogoutClicked: {
            screen.logout();
        }
        onCustomerServiceClicked: {
            stack.pop(null); // First go back to map page
            screen.showPopup(customerServicePopup);
        }
    }

    LoginPage {
        id: loginPage
        objectName: "loginPage"
        onCloseClicked: { stack.pop(); }
//        onSignupClicked: { stack.push(signupPage); }
    }

//    SignupPage {
//        id: signupPage
//        objectName: "signupPage"
//        onCloseClicked: { stack.pop(); }
//    }

    LogsPopup {
        id: logsPopup
        visible: false
        z: 20

        onCopy: {
            // Copy logs content to clipboard
            screen.hidePopup(logsPopup);
            mainwindow.copyLogsToClipboard();
        }

        onCancel: {
            screen.hidePopup(logsPopup);
        }
    }

    ErrorPopup {
        id: errorPopup
        visible: false
        title: qsTr("Error");
        z: 20

        onCancel: {
            screen.hidePopup(errorPopup);
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
            width: 240
            height: 240
            source: "../images/large-logo.png"
        }

        Text {
            anchors.top: splashLogo.bottom
            anchors.topMargin: 24
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Establishing a secure connection...");
            font.family: "Roboto"
            font.pixelSize: 20
            color: defaultColor
        }
    }

    StackView {
        id: stack
        initialItem: mapPage
        anchors.fill: screen

        popEnter: Transition {
                  XAnimator {
                      from: (stack.mirrored ? -1 : 1) * -stack.width
                      to: 0
                      duration: 2000
                      easing.type: Easing.OutCubic
                  }
              }

        popExit: Transition {
          XAnimator {
              from: 0
              to: (stack.mirrored ? -1 : 1) * stack.width
              duration: 2000
              easing.type: Easing.OutCubic
          }
        }

        pushEnter: Transition {
                  XAnimator {
                      from: (stack.mirrored ? -1 : 1) * stack.width
                      to: 0
                      duration: 2000
                      easing.type: Easing.OutCubic
                  }
              }

        pushExit: Transition {
          XAnimator {
              from: 0
              to: (stack.mirrored ? -1 : 1) * -stack.width
              duration: 2000
              easing.type: Easing.OutCubic
          }
        }
    }
}
