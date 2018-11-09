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
    id: selectionPopup
    anchors.bottom: parent.bottom
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.bottomMargin: 20
    spacing: 8

    property string title
    property string subtitle
    property variant itemModel
    property int selectedIndex

    signal cancel()
    signal itemSelected(int index)

    ButtonGroup {
        id: buttonGroup
    }

    Component {
        id: selectionDelegate
        Item {
            width: 295
            height: 48
            anchors.left: parent.left

            Rectangle {
                id: itemRect
                width: 295
                height: 48
                anchors.leftMargin: 20
                anchors.left: parent.left
                color: "white"
                radius: 5

                RadioButton {
                    id: radioButton
                    text: modelData
                    checked: selectionPopup.selectedIndex == index
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width
                    indicator: Image {
                              x: radioButton.leftPadding
                              y: parent.height / 2 - height / 2
                              source: radioButton.checked ? "../images/checked-radio-button.png" : "../images/empty-radio-button.png"
                          }

                    contentItem: Text {
                              text: radioButton.text
                              font.family: "Nunito-SemiBold"
                              font.pixelSize: 16
                              opacity: 1
                              color: "#020433"
                              verticalAlignment: Text.AlignVCenter
                              leftPadding: radioButton.indicator.width + radioButton.spacing
                          }

                    ButtonGroup.group: buttonGroup

                    onCheckedChanged: {
                        if (checked)
                            selectionPopup.selectedIndex = index;
                    }
                }

                layer.enabled: true
                layer.effect: DropShadow {
                    horizontalOffset: 1
                    verticalOffset: 1
                    color: "#80000000"
                }
            }
        }
    }

    Rectangle {
        id: itemSelectionBox

        width: 335
        height: 450
        color: "white"
        radius: 5

        Column {
            id: selectionColumn
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 36
            anchors.top: parent.top
            spacing: 0

            Text {
                font.family: "Roboto"
                font.bold: true
                font.pixelSize: 20
                color: defaultColor
                text: selectionPopup.title
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                font.family: "Roboto"
                font.bold: true
                font.pixelSize: 16
                color: "#6C798F"
                text: selectionPopup.subtitle
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Item {
                height: 24
                width: parent.width
            }

            ListView {
                width: parent.width
                height: 300
                delegate: selectionDelegate
                model: itemModel
                spacing: 8
            }
        }

        layer.enabled: true
        layer.effect: DropShadow {
            horizontalOffset: 1
            verticalOffset: 1
            color: "#80000000"
        }

    } // End of rect

    Rectangle {
        id: saveButton
        width: 335
        height: 48
        color: "#2CC532"
        radius: 5

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            font.family: "Roboto"
            font.bold: true
            font.pixelSize: 16
            text: qsTr("Save");
            color: "white"
        }

        MouseArea {
            cursorShape: Qt.PointingHandCursor
            anchors.fill: parent
            onClicked: {
                selectionPopup.itemSelected(selectionPopup.selectedIndex);
            }
        }

        layer.enabled: true
        layer.effect: DropShadow {
            horizontalOffset: 1
            verticalOffset: 1
            color: "#80000000"
        }
    }

    Rectangle {
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
            onClicked: { selectionPopup.cancel(); }
        }

        layer.enabled: true
        layer.effect: DropShadow {
            horizontalOffset: 1
            verticalOffset: 1
            color: "#80000000"
        }
    }

} // End of Item
