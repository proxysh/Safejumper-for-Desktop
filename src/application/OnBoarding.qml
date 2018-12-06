import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

Rectangle {
    id: onboardingScreen
    color: white

    property int page: 1

    signal gotoLogin();

    function showPage1() {
        page = 1;
        if (stack.currentItem.objectName != "page1")
            stack.pop(page1);
    }

    function showPage2() {
        page = 2
        if (stack.currentItem.objectName != "page4")
            stack.push(page4);
    }

    function showPage3() {
        page = 3
        if (stack.currentItem.objectName != "page3")
            stack.push(page3);
    }

    function nextPage() {
        if (page == 1) {
            showPage2();
        } else if (page == 2) {
            showPage3();
        } else {
            showPage1();
        }
    }

    Timer
    {
        interval: 20000
        running: true
        repeat: true
        onTriggered: { onboardingScreen.nextPage(); }
    }

    Rectangle {
        id: page1
        objectName: "page1"

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: { nextPage(); }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 123
            }

            Image {
                Layout.preferredWidth: 280
                Layout.preferredHeight: 256
                source: "../images/onboarding1.png"
                Layout.alignment: Qt.AlignHCenter
            }

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 69
            }

            Text {
                text: qsTr("Protect your Internet");
                font.family: "Roboto"
                font.weight: bold
                font.pixelSize: 21
                Layout.alignment: Qt.AlignHCenter
                color: "#172B4D"
            }

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 16
            }

            Text {
                text: qsTr("Your VPN account will allow you to secure your Internet connection and make you switch countries online.");
                font.family: "Roboto"
                font.pixelSize: 16
                color: "#7A869A"
                lineHeight: 26
                lineHeightMode: Text.FixedHeight
                horizontalAlignment: Qt.AlignHCenter
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 275
                wrapMode: Text.WordWrap
            }
        }
    }

    Rectangle {
        id: page2
        objectName: "page2"

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: { nextPage(); }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 93
            }

            Image {
                Layout.preferredWidth: 329
                Layout.preferredHeight: 300
                source: "../images/onboarding2.png"
                Layout.alignment: Qt.AlignHCenter
            }

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 69
            }

            Text {
                text: qsTr("Secure & anonymous");
                font.family: "Roboto"
                font.weight: bold
                font.pixelSize: 21
                Layout.alignment: Qt.AlignHCenter
                color: "#172B4D"
            }

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 16
            }

            Text {
                text: qsTr("Browse with confidence while you're connected to public wifi and other untrusted networks.");
                font.family: "Roboto"
                font.pixelSize: 16
                color: "#7A869A"
                lineHeight: 26
                lineHeightMode: Text.FixedHeight
                horizontalAlignment: Qt.AlignHCenter
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 275
                wrapMode: Text.WordWrap
            }
        }
    }

    Rectangle {
        id: page3
        objectName: "page3"

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: { nextPage(); }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 127
            }

            Image {
                Layout.preferredWidth: 329
                Layout.preferredHeight: 277
                source: "../images/onboarding3.png"
                Layout.alignment: Qt.AlignHCenter
            }

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 69
            }

            Text {
                text: qsTr("Unblock any content");
                font.family: "Roboto"
                font.weight: bold
                font.pixelSize: 21
                Layout.alignment: Qt.AlignHCenter
                color: "#172B4D"
            }

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 16
            }

            Text {
                text: qsTr("Ultra fast VPN servers across 94 countries. Unblock the sites and apps you love, instantly.");
                font.family: "Roboto"
                font.pixelSize: 16
                color: "#7A869A"
                lineHeight: 26
                lineHeightMode: Text.FixedHeight
                horizontalAlignment: Qt.AlignHCenter
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 275
                wrapMode: Text.WordWrap
            }
        }
    }

    Rectangle {
        id: page4
        objectName: "page4"

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: { nextPage(); }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 160
            }

            Image {
                Layout.preferredWidth: 348
                Layout.preferredHeight: 240
                source: "../images/onboarding4.png"
                Layout.alignment: Qt.AlignHCenter
            }

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 69
            }

            Text {
                text: qsTr("Control your assets");
                font.family: "Roboto"
                font.weight: bold
                font.pixelSize: 21
                Layout.alignment: Qt.AlignHCenter
                color: "#172B4D"
            }

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 16
            }

            Text {
                text: qsTr("Make online payments safely and securely. Your card and account information is safe and secure.");
                font.family: "Roboto"
                font.pixelSize: 16
                color: "#7A869A"
                lineHeight: 26
                lineHeightMode: Text.FixedHeight
                horizontalAlignment: Qt.AlignHCenter
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 275
                wrapMode: Text.WordWrap
            }
        }
    }

    Rectangle {
        id: page5
        objectName: "page5"

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: { nextPage(); }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight:160
            }

            Image {
                Layout.preferredWidth: 329
                Layout.preferredHeight: 230
                source: "../images/onboarding5.png"
                Layout.alignment: Qt.AlignHCenter
            }

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 69
            }

            Text {
                text: qsTr("24/7 customer service");
                font.family: "Roboto"
                font.weight: bold
                font.pixelSize: 21
                Layout.alignment: Qt.AlignHCenter
                color: "#172B4D"
            }

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 16
            }

            Text {
                text: qsTr("Our award-winning customer service agents are ready to help you all day, every day.");
                font.family: "Roboto"
                font.pixelSize: 16
                color: "#7A869A"
                lineHeight: 26
                lineHeightMode: Text.FixedHeight
                horizontalAlignment: Qt.AlignHCenter
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 275
                wrapMode: Text.WordWrap
            }
        }
    }

    Rectangle {
        id: page6
        objectName: "page6"

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: { nextPage(); }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 113
            }

            Image {
                Layout.preferredWidth: 329
                Layout.preferredHeight: 277
                source: "../images/onboarding6.png"
                Layout.alignment: Qt.AlignHCenter
            }

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 69
            }

            Text {
                text: qsTr("100% non-logging");
                font.family: "Roboto"
                font.weight: bold
                font.pixelSize: 21
                Layout.alignment: Qt.AlignHCenter
                color: "#172B4D"
            }

            Item {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 16
            }

            Text {
                text: qsTr("We don't log any activity of customers connected to our service. Period. Your privacy is paramount.");
                font.family: "Roboto"
                font.pixelSize: 16
                color: "#7A869A"
                lineHeight: 26
                lineHeightMode: Text.FixedHeight
                horizontalAlignment: Qt.AlignHCenter
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 275
                wrapMode: Text.WordWrap
            }
        }
    }


    StackView {
        id: stack
        width: parent.width
        height: 600
        anchors.top: parent.top
        initialItem: page1

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
    }

    RowLayout {
        width: 68
        height: 12
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: stack.bottom
        anchors.topMargin: 30
        spacing: 10

        Image {
            id: button1
            source: (page == 1 ? "../images/filledcircle.png" : "../images/emptycircle.png")

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: { showPage1(); }
            }
        }

        Image {
            id: button2
            source: (page == 2 ? "../images/filledcircle.png" : "../images/emptycircle.png")

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: { showPage2(); }
            }
        }

        Image {
            id: button3
            source: (page == 3 ? "../images/filledcircle.png" : "../images/emptycircle.png")

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: { showPage3(); }
            }
        }
    }

    Button {
        id: loginButton
        width: 295
        height: 56
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            acceptedButtons: Qt.NoButton
        }

        background: ShadowRect {
            opacity: enabled ? 1 : 0.3
            border.color: defaultColor
            border.width: 2
            color: white
            radius: 5
        }

        contentItem: Text {
            opacity: enabled ? 1.0 : 0.3
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 16
            font.family: "Roboto"
            font.bold: true
            color: defaultColor
            text: qsTr("LOGIN");
        }

        onClicked: {
            onboardingScreen.gotoLogin();
        }
    }
}
