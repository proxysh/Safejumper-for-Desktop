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

Item {
    id: allServersPage

    signal menuClicked()
    signal settingsClicked()
    signal allServersClicked()

    Rectangle {
        id: guiRectangle
        color: "#F4F5F7"
        anchors.fill: parent

        HeaderBar {
            id: headerArea
            onSettingsClicked: { allServersPage.settingsClicked(); }
            onMenuClicked: { allServersPage.menuClicked(); }
        }

        ListView {
            id: allServersList
            anchors.top: headerArea.bottom
            anchors.bottom: parent.bottom
            width: parent.width

            model: serversModel
            spacing: 10
            clip: true
            delegate:
                ServerCard {
                    currentServer: model.server(index)

            }
        }
    }
}
