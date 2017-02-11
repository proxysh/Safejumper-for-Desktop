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

    int currentServerId();	  // -1 if not selected, otherwise [0-...] id of server inside auth manager
    int serverIndexFromLineIndex(int row_id);		// [0, size()) , omit first row ' -- select location -- '
    bool useServerColumn();
    int currentProtocol();	// -1 if not selected, otherwise [0-...] id of protocol inside Settings

    void repopulateProtocols();
    void setServer(int ixsrv);
    void setProtocol(int ix);
    void statusConnecting();
    void statusConnected();
    void statusDisconnected();

    void switchToNextNode();

public slots:
    void repopulateLocations();
    void repopulateLocations(bool random);		// TODO: -1 methods to update particular row with new ping/load%

protected:
    virtual void closeEvent(QCloseEvent * event);
    virtual void keyPressEvent(QKeyEvent * event);

private slots:
    void on_backButton_clicked();

    void on_protocolComboBox_currentIndexChanged(int ix);
    void on_locationComboBox_currentIndexChanged(int ix);

    void on_settingsButton_clicked();
    void on_connectButton_clicked();

    void protocolChanged(); // slot for reacting to Setting protocolChanged signal
private:
    void setRowStyle(bool show_nodes);
    void displayMark(const QString & name);

    Ui::MapScreen *ui;
    static std::auto_ptr<MapScreen> mInstance;
    explicit MapScreen(QWidget *parent = 0);
    bool mShowingNodes;		// remember for which srv/hub the list was populated; in settings maybe inadequate due to refill during settings change
    int mEncryption;				// remember encryption when list was populated
    bool mUseServerColumn;
    std::vector<int> mServerIds;
    QPoint mDefaultPoint;
    bool mMoving;
    QPoint mWindowStartPoint;
    QPoint mCursorStartPoint;
    bool mRepopulationInProgress;
    QPoint mCurrentPoint;

};

#endif // MAPSCREEN_H
