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

#include "mapscreen.h"

#include <QListView>
#include <QItemDelegate>
#include <QStandardItemModel>

#include "setting.h"
#include "ui_mapscreen.h"
#include "scr_settings.h"
#include "wndmanager.h"
#include "authmanager.h"
#include "openvpnmanager.h"
#include "log.h"
#include "flag.h"
#include "locationdelegate.h"
#include "protocoldelegate.h"
#include "fonthelper.h"
#include "loginwindow.h"

MapScreen::MapScreen(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapScreen),
    mShowingNodes(true),
    mEncryption(0),
    mUseServerColumn(true),
    mRepopulationInProgress(false)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());
#ifndef Q_OS_DARWIN
    FontHelper::SetFont(this);
#endif

#ifdef Q_OS_WIN
    ui->lv_Location->setMinimumWidth(300);
    ui->lv_Location->setMinimumHeight(400);
#endif
    mDefaultPoint = ui->L_Mark->pos();

    ui->locationComboBox->setView(ui->lv_Location);
    ui->protocolComboBox->setView(ui->lv_Protocol);
    ui->protocolComboBox->setItemDelegate(new ProtocolDelegate(this));
    ui->locationComboBox->setItemDelegate(new LocationDelegate(this));

    repopulate();

//	setMouseTracking(true); // E.g. set in your constructor of your widget.
    connect(Setting::instance(), &Setting::showNodesChanged,
            this, &MapScreen::repopulateLocations);

    connect(Setting::instance(), &Setting::encryptionChanged,
            this, &MapScreen::repopulate);
    connect(Setting::instance(), &Setting::protocolChanged,
            this, &MapScreen::updateProtocol);
    connect(Setting::instance(), &Setting::serverChanged,
            this, &MapScreen::updateLocation);
}


static const QString gs_Individual = "QListView\n{\nbackground: solid #f7f7f7;\n}\nQListView::item\n{\n/*color: #202020;*/\ncolor: #f7f7f7;	/*invisible*/\nheight: 24;\nborder-image: url(:/imgs/dd-row.png);\nbackground: solid #f7f7f7;\n}\nQListView::item:selected\n{\nborder-image: url(:/imgs/dd-selectionrow-322.png);\n}\n\n";
static const QString gs_Hubs = "QListView\n{\nbackground: solid #f7f7f7;\n}\nQListView::item\n{\n/*color: #202020;*/\ncolor: #f7f7f7;	/*invisible*/\nheight: 24;\nborder-image: url(:/imgs/dd-row.png);\nbackground: solid #f7f7f7;\n}\nQListView::item:selected\n{\nborder-image: url(:/imgs/dd-selectionrow-244.png);\n}\n\n";
void MapScreen::setRowStyle(bool show_nodes)
{
    if (show_nodes)
        ui->lv_Location->setStyleSheet(gs_Individual);
    else
        ui->lv_Location->setStyleSheet(gs_Hubs);
}

void MapScreen::repopulateProtocols()
{
    mRepopulationInProgress = true;

    const std::vector<QString> & v = Setting::instance()->currentEncryptionProtocols();
    ui->protocolComboBox->clear();
    ui->protocolComboBox->addItem(PROTOCOL_SELECTION_STR);
    for (size_t k = 0; k < v.size(); ++k)
        ui->protocolComboBox->addItem(v.at(k));

    mRepopulationInProgress = false;

    if (v.size() == 1) {
        ui->protocolComboBox->setCurrentIndex(1);
        ui->protocolComboBox->setEnabled(false);
    } else {
        ui->protocolComboBox->setEnabled(true);
    }

    updateProtocol();
}

void MapScreen::repopulateLocations()
{
    mRepopulationInProgress = true;

    // store previously chosen id
    int oldN = ui->locationComboBox->count();
    int ixoldsrv = -1;
    QString oldsrv;
    bool oldShownodes = mShowingNodes;
//    bool oldUsesrvcoll = mUseServerColumn;
//    int oldEnc = mEncryption;
    if (oldN > 1) {
        ixoldsrv = currentServerId();
        if (ixoldsrv > -1) {
            oldsrv = AuthManager::instance()->getServer(ixoldsrv).name;
        }
        // and clear the list
        ui->locationComboBox->setCurrentIndex(0);
        // Remove all items except "--Select Location--"
        for (int k = oldN; k > 0; --k)
            ui->locationComboBox->removeItem(k);
    }

    // populate with the actual servers
    mShowingNodes = Setting::instance()->showNodes();
    mEncryption = Setting::instance()->encryption();
    mUseServerColumn = ((mEncryption == ENCRYPTION_RSA) && mShowingNodes) || (mEncryption > ENCRYPTION_RSA);

    setRowStyle(mUseServerColumn);

    const QList<int> & coll = (mUseServerColumn ?
                               AuthManager::instance()->currentEncryptionServers() :
                               AuthManager::instance()->currentEncryptionHubs() );

    // populate server ids to show
    mServerIds = coll;

    // for each server id create a line
    for (int j = 0; j < mServerIds.size(); ++j) {
        // add individual srv / hub node item
        // TODO: -1 format the line
        int ix = mServerIds.at(j);
        AServer srv = AuthManager::instance()->getServer(ix);
        int ping = AuthManager::instance()->pingFromServerIx(ix);
        QString s0 = srv.name;
        QString s1;
        int load = (int)srv.load.toDouble();

        s1 += QString::number(load) + "%";
        if (ping > -1)
            s1 += " / " + QString::number(ping) + "ms";
        s0 += "\t\t";				// HACK: override width
        ui->locationComboBox->addItem(s0);
    }

// TODO: -0 for other encryptions does not work
    // try to reselect the chosen server
    int toselect = 0;
    if (oldN > 1) { // Used to have some servers
        if (ixoldsrv > -1) { // Used to have a selected server
            QString newname;
            int new_row = -1;
            for (int k = 0; k < mServerIds.size(); ++k) {
                if (mServerIds.at(k) == ixoldsrv) {
                    // srv id match: ensure this is the same server after list updated
                    if (oldsrv == AuthManager::instance()->getServer(mServerIds.at(k)).name) {
                        new_row = k;
                        break;
                    }
                }
            }

            if (new_row == -1) {
                // lookup by name
                int ixnew = AuthManager::instance()->serverIxFromName(oldsrv);
                if (ixnew > -1) {
                    for (int k = 0; k < mServerIds.size(); ++k) {
                        if (mServerIds.at(k) == ixnew) {
                            new_row = k;
                            break;
                        }
                    }
                }
            }

            if (new_row == -1) {
                // lookup by name
                if (!mShowingNodes && oldShownodes) {
                    // all -->> hubs
                    int newhubix = AuthManager::instance()->hubIxFromServerName(oldsrv);
                    if (newhubix > -1) {
                        for (int k = 0; k < mServerIds.size(); ++k) {
                            if (mServerIds.at(k) == newhubix) {
                                new_row = k;
                                break;
                            }
                        }
                    }
                }
            }
            if (new_row != -1)
                toselect = new_row + 1;
            else
                toselect = qrand() % ui->locationComboBox->count() + 1;
        }
    }
    mRepopulationInProgress = false;

    ui->locationComboBox->setCurrentIndex(toselect);
    updateLocation();
}

int MapScreen::serverIndexFromLineIndex(int row_id)
{
    int ixsrv = -1;
    if (row_id > -1 && row_id < mServerIds.size())
        ixsrv = mServerIds.at(row_id);
    return ixsrv;
}

bool MapScreen::useServerColumn()
{
    return mUseServerColumn;
}

void MapScreen::closeEvent(QCloseEvent * event)
{
    event->ignore();
    WndManager::Instance()->HideThis(this);
}

MapScreen::~MapScreen()
{
    delete ui;
}

std::auto_ptr<MapScreen> MapScreen::mInstance;
MapScreen * MapScreen::instance()
{
    if (!mInstance.get())
        mInstance.reset(new MapScreen());
    return mInstance.get();
}

bool MapScreen::exists()
{
    return (mInstance.get() != NULL);
}

void MapScreen::cleanup()
{
    if (mInstance.get() != NULL)
        delete mInstance.release();
}

int MapScreen::currentServerId()
{
    int srv = -1;
    int ix = ui->locationComboBox->currentIndex();
    if(ix > 0) {
        int id = ix -1;
        if (!mServerIds.empty() && id < mServerIds.size()) {
            srv = mServerIds.at(id);
        }
        /*
        if (_IsShowNodes)
            srv = ix - 1;   // TODO: -0 inadequate during repopulation during changes of number of servers
        else
            srv = AuthManager::Instance()->ServerIdFromHubId(ix - 1);
        */
    }
    return srv;
}

void MapScreen::on_backButton_clicked()
{
    WndManager::Instance()->ToPrimary();
}

void MapScreen::on_settingsButton_clicked()
{
    WndManager::Instance()->ToSettings();
}

void MapScreen::on_connectButton_clicked()
{
    OpenvpnManager::instance()->start();
}

void MapScreen::repopulate()
{
    // Repopulate protocols
    repopulateProtocols();
    // Repopulate locations
    repopulateLocations();
}

void MapScreen::updateProtocol()
{
    ui->protocolComboBox->setCurrentIndex(Setting::instance()->currentProtocol() + 1);
}

void MapScreen::updateLocation()
{
    int ixsrv = Setting::instance()->serverID();
    int toselect = 0;
    const QList<int> & srvs = AuthManager::instance()->currentEncryptionServers();
    if (ixsrv > -1) {
        if (mEncryption == ENCRYPTION_RSA) {
            if (mShowingNodes)
                toselect = ixsrv + 1;
            else
                toselect = AuthManager::instance()->hubIdFromServerId(ixsrv) + 1;
        } else {
            if (!srvs.isEmpty()) {
                for (int k = 0; k < srvs.size(); ++k) {
                    if (srvs[k] == ixsrv) {
                        toselect = k + 1;
                        break;
                    }
                }
            }
        }
    }
    if (!srvs.empty())
        ui->locationComboBox->setCurrentIndex(toselect);
}

static const char * const gs_stIcon1 = "QLabel\n{\n	border:0px;\n	color: #ffffff;\nborder-image: url(:/imgs/l-1.png);\n}";
static const char * const gs_stIcon2 = "QLabel\n{\n	border:0px;\n	color: #ffffff;\nborder-image: url(:/imgs/l-2.png);\n}";
static const char * const gs_stIcon2inact = "QLabel\n{\n	border:0px;\n	color: #ffffff;\nborder-image: url(:/imgs/l-2-inactive.png);\n}";
static const char * const gs_stIconV = "QLabel\n{\n	border:0px;\n	color: #ffffff;\nborder-image: url(:/imgs/l-v.png);\n}";

void MapScreen::on_protocolComboBox_currentIndexChanged(int ix)
{
    if (mRepopulationInProgress)
        return;

    if (ix > 0) {
        ui->locationComboBox->setEnabled(true);
        ui->L_1->setStyleSheet(gs_stIconV);
        if (ui->locationComboBox->currentIndex() > 0)
            ui->L_2->setStyleSheet(gs_stIconV);
        else
            ui->L_2->setStyleSheet(gs_stIcon2);
    } else {
        ui->locationComboBox->setEnabled(false);
        ui->L_1->setStyleSheet(gs_stIcon1);
        ui->L_2->setStyleSheet(gs_stIcon2inact);
    }
    Setting::instance()->setProtocol(ix - 1);
}

void MapScreen::on_locationComboBox_currentIndexChanged(int ix)
{
    if (mRepopulationInProgress)
        return;

    int ixsrv = -1;
    if (ix > 0) {
        ui->L_2->setStyleSheet(gs_stIconV);
        ixsrv = currentServerId();
        AuthManager::instance()->setNewIp("");
    } else {
        ui->L_2->setStyleSheet(gs_stIcon2);
    }
    AServer se = AuthManager::instance()->getServer(ixsrv);
    QString newsrv = se.name;
    Setting::instance()->setServer(ixsrv);

    displayMark(se.name);
}

void MapScreen::displayMark(const QString & name)
{
    QPoint p = flag::CoordsFromSrvName(name);
    if (p.x() == 0 && p.y() == 0)
        p = mDefaultPoint;
    ui->L_Mark->move(p);
    ui->L_Mark->setText(flag::ShortName(name));
}

void MapScreen::statusConnecting()
{
    ui->connectButton->setEnabled(false);
    // TODO: -1 CANCEL button
}

void MapScreen::statusConnected()
{
    ;
}

void MapScreen::statusDisconnected()
{
    ui->connectButton->setEnabled(true);
}

void MapScreen::keyPressEvent(QKeyEvent * e)
{
    if(e->key() != Qt::Key_Escape)
        QDialog::keyPressEvent(e);
}
