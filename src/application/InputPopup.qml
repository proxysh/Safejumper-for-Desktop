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
    id: inputPopup
    anchors.bottom: parent.bottom
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.bottomMargin: 20
    spacing: 8

    property string title
    property string subtitle
    property string value

    signal cancel()
    signal inputSaved(int value)

    Rectangle {
        width: 335
        // Add 36 for top margin and 16 for bottom margin
        height: inputColumn.childrenRect.height + 36 + 16
        color: "white"
        radius: 5

        Column {
            id: inputColumn
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.right: parent.right
            anchors.rightMargin: 20
            anchors.topMargin: 36
            anchors.top: parent.top
            spacing: 0

            Text {
                font.family: "Roboto"
                font.bold: true
                font.pixelSize: 20
                color: defaultColor
                text: inputPopup.title
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                font.family: "Roboto"
                font.bold: true
                font.pixelSize: 16
                width: 295
                wrapMode: Text.WordWrap
                color: "#6C798F"
                text: inputPopup.subtitle
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Item {
                height: 24
                width: parent.width
            }

            Rectangle {
                width: 295
                height: 56

                color: "white"
                radius: 5

                layer.enabled: true
                layer.effect: DropShadow {
                    horizontalOffset: 1
                    verticalOffset: 1
                    color: "#80000000"
                }

                Column {
                    anchors.fill: parent
                    anchors.topMargin: 8
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 4

                    Text {
                        font.pixelSize: 12
                        font.family: "Roboto-Black"
                        color: "#6C798F"
                        text: qsTr("PORT");
                    }

                    TextField {
                        id: inputBox
                        width: parent.width
                        leftPadding: 0
                        placeholderText: qsTr("Enter value");
                        font.pixelSize: 16
                        color: "#97A0AF"
                        font.family: "Roboto"
                        font.bold: true

                        text: inputPopup.value

                        background: Rectangle {
                                  color: "transparent"
                                  border.color: "transparent"
                              }
                    }
                }
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
                inputPopup.inputSaved(inputBox.text);
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
            onClicked: { inputPopup.cancel(); }
        }

        layer.enabled: true
        layer.effect: DropShadow {
            horizontalOffset: 1
            verticalOffset: 1
            color: "#80000000"
        }
    }

} // End of Item
