/***************************************************************************
 *   Copyright (C) 2017 by Jeremy Whiting <jeremypwhiting@gmail.com>       *
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

#ifndef MAPSCREEN_H
#define MAPSCREEN_H

#include "common.h"

#include <memory>

#include <QDialog>
#include <QCloseEvent>

namespace Ui
{
class MapScreen;
}

#define PROTOCOL_SELECTION_STR "-- Select protocol & port --"

class MapScreen : public QDialog
{
    Q_OBJECT

public:
    static MapScreen * instance();
    static bool exists();
    static void cleanup();
    ~MapScreen();

    int serverIndexFromLineIndex(int row_id);		// [0, size()) , omit first row ' -- select location -- '
    bool useServerColumn();

    void statusConnecting();
    void statusConnected();
    void statusDisconnected();

protected:
    virtual void closeEvent(QCloseEvent * event);
    virtual void keyPressEvent(QKeyEvent * event);

private slots:
    void on_backButton_clicked();

    void on_protocolComboBox_currentIndexChanged(int ix);
    void on_locationComboBox_currentIndexChanged(int ix);

    void on_settingsButton_clicked();
    void on_connectButton_clicked();

    void repopulate(); // Slot for when encryption changes so both location and protocol need to be repopulated
    void repopulateProtocols();
    void repopulateLocations();

    void updateProtocol(); // slot for reacting to Setting protocolChanged signal
    void updateLocation(); // slot for reacting to Setting locationChanged signal

    void stateChanged(vpnState state);
private:
    int currentServerId();	  // -1 if not selected, otherwise [0-...] id of server inside auth manager
    void setRowStyle(bool show_nodes);
    void displayMark(const QString & name);

    Ui::MapScreen *ui;
    static std::auto_ptr<MapScreen> mInstance;
    explicit MapScreen(QWidget *parent = 0);
    bool mShowingNodes;		// remember for which srv/hub the list was populated; in settings maybe inadequate due to refill during settings change
    int mEncryption;				// remember encryption when list was populated
    bool mUseServerColumn;
    QList<int> mServerIds;
    QPoint mDefaultPoint;
    bool mRepopulationInProgress;
};

#endif // MAPSCREEN_H
