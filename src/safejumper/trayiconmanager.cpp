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

#include "trayiconmanager.h"

#include <QMenu>
#include <QFontDatabase>
#include <QTimer>

#include "scr_settings.h"
#include "scr_logs.h"
#include "scr_map.h"
#include "dlg_error.h"
#include "confirmationdialog.h"

#ifdef MONITOR_TOOL
#include "scr_table.h"
#endif	// MONITOR_TOOL

#include "authmanager.h"
#include "wndmanager.h"
#include "common.h"
#include "version.h"
#include "setting.h"

#include "openvpnmanager.h"

#include "osspecific.h"
#include "log.h"

TrayIconManager::TrayIconManager(QWidget *parent)
    :mFixed(false),
    mActivatedCount(0),
    mParent(parent)
{
    createTrayIcon();
    mTrayIcon->show();

    statusDisconnected();

    disableActionsOnLogout();
}

bool TrayIconManager::exists()
{
    return (mInstance.get() != NULL);
}

void TrayIconManager::disableActionsOnLogout()
{
    mLogoutAction->setEnabled(false);	//_ac_Logout->setIcon(QIcon(":/icons-tm/close-grey.png"));
    mJumpAction->setEnabled(false);	//_ac_Jump->setIcon(QIcon(":/icons-tm/jump-grey.png"));
    mSwitchCountryAction->setEnabled(false);	//_ac_SwitchCountry->setIcon(QIcon(":/icons-tm/country-grey.png"));
}

void TrayIconManager::enableButtonsOnLogin()
{
    mLogoutAction->setEnabled(true);	//_ac_Logout->setIcon(QIcon(":/icons-tm/close-red.png"));
    mJumpAction->setEnabled(true);		//_ac_Jump->setIcon(QIcon(":/icons-tm/jump-red.png"));
    mSwitchCountryAction->setEnabled(true);	//_ac_SwitchCountry->setIcon(QIcon(":/icons-tm/country-red.png"));
}

TrayIconManager::~TrayIconManager()
{
    clearConnectToMenu();
    if (mConnectToMenu.get()) {
        if (mTrayIconMenu.get()) {
// TODO: -2			_TrayMenu->removeAction(_ct_menu.get());
        }
    }
}

void TrayIconManager::createTrayIcon()
{
    mTrayIcon.reset(new QSystemTrayIcon(mParent));
    createTrayIconMenu();
    QMenu *menu = mTrayIconMenu.get();
    mTrayIcon->setContextMenu(menu);
    QIcon icon(OsSpecific::Instance()->IconDisconnected());
    mTrayIcon->setIcon(icon);
    connect(mTrayIcon.get(), SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(actionActivated(QSystemTrayIcon::ActivationReason)));
}

void TrayIconManager::createTrayIconMenu()
{
    mTrayIconMenu.reset(new QMenu(mParent));
    createMenuActions();

    mTrayIconMenu->addAction(mConnectAction.get());
    mTrayIconMenu->addAction(mConnectToAction.get());
    mTrayIconMenu->addAction(mDisconnectAction.get());
    mTrayIconMenu->addSeparator();

    mTrayIconMenu->addAction(mStatusAction.get());
    mTrayIconMenu->addSeparator();

    mTrayIconMenu->addAction(mJumpAction.get());
    mTrayIconMenu->addAction(mSwitchCountryAction.get());
    mSwitchCountryAction->setEnabled(false);
    mTrayIconMenu->addSeparator();

    mTrayIconMenu->addAction(mSettingsAction.get());
    mTrayIconMenu->addAction(mLogsAction.get());
    mTrayIconMenu->addSeparator();

    mTrayIconMenu->addAction(mWebManageAction.get());
    mTrayIconMenu->addAction(mSupportAction.get());
    mTrayIconMenu->addAction(mBugAction.get());
    mTrayIconMenu->addAction(mEarnAction.get());
    mTrayIconMenu->addSeparator();

    //_TrayMenu->addAction(_ac_About.get());
    mTrayIconMenu->addAction(mLogoutAction.get());
    mTrayIconMenu->addAction(mCloseAction.get());
}

void TrayIconManager::createMenuActions()
{
    mConnectAction.reset(new QAction(//QIcon(":/icons-tm/connect-red.png"),
                          tr("&Connect"), this));
    connect(mConnectAction.get(), SIGNAL(triggered()), this, SLOT(connectTriggered()));

    mConnectToAction.reset(new QAction(//QIcon(":/icons-tm/connect-grey.png"),
                            tr("&Connect to ..."), this));
    mConnectToAction->setEnabled(false);
    //connect(_ac_ConnectTo.get(), SIGNAL(triggered()), this, SLOT(ac_ConnectTo()));

    mDisconnectAction.reset(new QAction(//QIcon(":/icons-tm/disconnect-grey.png"),
                             tr("&Disconnect"), this));
    connect(mDisconnectAction.get(), SIGNAL(triggered()), this, SLOT(disconnectTriggered()));

    mStatusAction.reset(new QAction(//QIcon(":/icons-tm/status-red.png"),
                         tr("&Status"), this));
    connect(mStatusAction.get(), SIGNAL(triggered()), this, SLOT(statusTriggered()));

    mJumpAction.reset(new QAction(//QIcon(":/icons-tm/status-grey.png"),
                       tr("&Jump to Faster"), this));
    connect(mJumpAction.get(), SIGNAL(triggered()), this, SLOT(jumpTriggered()));

    mSwitchCountryAction.reset(new QAction(//QIcon(":/icons-tm/country-grey.png"),
                                tr("Switch Country"), this));
    connect(mSwitchCountryAction.get(), SIGNAL(triggered()), this, SLOT(switchCountryTriggered()));

    mSettingsAction.reset(new QAction(//QIcon(":/icons-tm/settings-red.png"),
                           tr("Se&ttings"), this));
    connect(mSettingsAction.get(), SIGNAL(triggered()), this, SLOT(settingsTriggered()));

    mLogsAction.reset(new QAction(//QIcon(":/icons-tm/logs-red.png"),
                       tr("&Logs"), this));
    connect(mLogsAction.get(), SIGNAL(triggered()), this, SLOT(logsTriggered()));

    mWebManageAction.reset(new QAction(//QIcon(":/icons-tm/webmanag-red.png"),
                           tr("&Web Management"), this));
    connect(mWebManageAction.get(), SIGNAL(triggered()), this, SLOT(webManagTriggered()));

    mSupportAction.reset(new QAction(//QIcon(":/icons-tm/support-red.png"),
                          tr("&Feedback/Support"), this));
    connect(mSupportAction.get(), SIGNAL(triggered()), this, SLOT(supportTriggered()));

    mBugAction.reset(new QAction(//QIcon(":/icons-tm/bug-red.png"),
                      tr("&Report Bug"), this));
    connect(mBugAction.get(), SIGNAL(triggered()), this, SLOT(bugTriggered()));

    mEarnAction.reset(new QAction(//QIcon(":/icons-tm/earn-red.png"),
                       tr("&Earn Money"), this));
    connect(mEarnAction.get(), SIGNAL(triggered()), this, SLOT(earnTriggered()));

    //_ac_About.reset(new QAction(tr("&About"), this));
    //connect(_ac_About.get(), SIGNAL(triggered()), this, SLOT(ac_About()));

    mLogoutAction.reset(new QAction(//QIcon(":/icons-tm/close-grey.png"),
                         tr("Logout"), this));
    connect(mLogoutAction.get(), SIGNAL(triggered()), this, SLOT(logoutTriggered()));

    mCloseAction.reset(new QAction(//QIcon(":/icons-tm/close-red.png"),
                        tr("Close"), this));
    connect(mCloseAction.get(), SIGNAL(triggered()), this, SLOT(closeTriggered()));
}

void TrayIconManager::actionActivated(QSystemTrayIcon::ActivationReason )
{
    ++mActivatedCount;
    if (mActivatedCount == 1 && !mFixed) {
        connect(g_pTheApp, SIGNAL(focusChanged(QWidget*, QWidget*)),
                this, SLOT(focusChanged(QWidget*, QWidget*)));
        if (mIconTimer.get() == NULL) {
            mIconTimer.reset(new QTimer(this));
            connect(mIconTimer.get(), SIGNAL(timeout()),
                    this, SLOT(updateStateIcon()));
            mIconTimer->start(210);
        }
        updateStateIcon();
    } else {
        disconnectIconWatcher();
        updateStateIcon();
    }
}

void TrayIconManager::disconnectIconWatcher()
{
    mFixed = true;
    disconnect(mTrayIcon.get(), SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
               this, SLOT(actionActivated(QSystemTrayIcon::ActivationReason)));
    disconnect(g_pTheApp, SIGNAL(focusChanged(QWidget*, QWidget*)),
               this, SLOT(focusChanged(QWidget*, QWidget*)));
    if (mIconTimer.get() != NULL)
        mIconTimer->stop();
}

void TrayIconManager::focusChanged(QWidget*, QWidget*)
{
    disconnectIconWatcher();
    updateStateIcon();
}

void TrayIconManager::updateStateIcon()
{
    OpenvpnManager::OvState st = OpenvpnManager::ovsDisconnected;
    if (OpenvpnManager::exists())
        st = OpenvpnManager::Instance()->state();
    updateStateIcon(st);
}

void TrayIconManager::updateStateIcon(OpenvpnManager::OvState st)
{
    QString ic;
    switch (st) {
    case OpenvpnManager::ovsDisconnected:
        ic = OsSpecific::Instance()->IconDisconnected();
        break;
    case OpenvpnManager::ovsConnecting:
        ic = OsSpecific::Instance()->IconConnecting();
        break;
    case OpenvpnManager::ovsConnected:
        ic = OsSpecific::Instance()->IconConnected();
        mJumpAction->setEnabled(true);
        mSwitchCountryAction->setEnabled(true);
        break;
    default:
        break;
    }
    QIcon icon(ic);
    mTrayIcon->setIcon(icon);
}

void TrayIconManager::fixIcon()
{
    if (!mFixed) {
        mFixed = true;
        disconnectIconWatcher();
        updateStateIcon();
    }
}

void TrayIconManager::connectTriggered()
{
    fixIcon();
    if (!AuthManager::Instance()->loggedIn()) {
        emit login();
    } else {
        OpenvpnManager::Instance()->start();
    }
}

void TrayIconManager::connectToTriggered()
{
    fixIcon();
    // Get the action from the sender
    QAction *action = qobject_cast<QAction*>(sender());
    if (action != NULL) {
        // Get the action's server id
        if (AuthManager::Instance()->loggedIn()) {
            size_t serverId = action->data().toInt();

            Scr_Map::Instance()->SetServer(serverId);
            OpenvpnManager::Instance()->start();
        } else {
            emit login();
        }
    }
}

void TrayIconManager::disconnectTriggered()
{
    fixIcon();
    OpenvpnManager::Instance()->stop();
}

void TrayIconManager::statusTriggered()
{
    fixIcon();
    WndManager::Instance()->ToPrimary();
}

void TrayIconManager::jumpTriggered()
{
    fixIcon();
    WndManager::Instance()->ToPrimary();
    AuthManager::Instance()->jump();
}

void TrayIconManager::switchCountryTriggered()
{
    fixIcon();
    WndManager::Instance()->ToMap();
}

void TrayIconManager::settingsTriggered()
{
    fixIcon();
    WndManager::Instance()->ToSettings();
}

void TrayIconManager::logsTriggered()
{
    fixIcon();
    WndManager::Instance()->ToLogs();
}

void TrayIconManager::webManagTriggered()
{
    fixIcon();
    WndManager::Instance()->CloseAll();
    OpenUrl_Panel();
}

void TrayIconManager::supportTriggered()
{
    fixIcon();
    WndManager::Instance()->CloseAll();
    OpenUrl_Support();
}

void TrayIconManager::bugTriggered()
{
    fixIcon();
    WndManager::Instance()->CloseAll();
    OpenUrl_Bug();
}

void TrayIconManager::earnTriggered()
{
    fixIcon();
    WndManager::Instance()->CloseAll();
    OpenUrl_Earn();
}

void TrayIconManager::aboutTriggered()
{
    fixIcon();
    WndManager::Instance()->ToPrimary();
}

void TrayIconManager::closeTriggered()
{
    fixIcon();
    emit quitApplication();
}

void TrayIconManager::logoutTriggered()
{
    fixIcon();
    if (OpenvpnManager::exists())
        OpenvpnManager::Instance()->stop();
    if (AuthManager::exists())
        AuthManager::Instance()->logout();
    WndManager::Instance()->ToPrimary();
    emit logout();
    clearConnectToMenu();
    mConnectToMenu->setEnabled(false);
    //_ct_menu->setIcon(QIcon(":/icons-tm/connect-grey.png"));
    disableActionsOnLogout();
}

std::auto_ptr<TrayIconManager> TrayIconManager::mInstance;
TrayIconManager * TrayIconManager::instance()
{
    if (!mInstance.get())
        mInstance.reset(new TrayIconManager(NULL));
    return mInstance.get();
}

void TrayIconManager::cleanup()
{
    std::auto_ptr<TrayIconManager> d(mInstance.release());
}

void TrayIconManager::createMenuItem(QMenu * m, const QString & name, size_t srv)
{
    QAction * a = m->addAction(name);
    a->setData(QVariant(int(srv)));
    connect(a, SIGNAL(triggered()),
            this, SLOT(connectToTriggered()));
}

void TrayIconManager::constructConnectToMenu()
{
    qDebug() << "creating connect to menu";
    if (AuthManager::exists()) {
        AuthManager * am = AuthManager::Instance();
        if (am->loggedIn()) {
            qDebug() << "Logged in so making actions";
            clearConnectToMenu();

            const std::vector<size_t> & hubs = am->currentEncryptionHubs();
            if (mConnectToMenu.get() == NULL) {	// one time during entire program run
                mConnectToMenu.reset(mTrayIconMenu->addMenu("Connect to ..."));
                mTrayIconMenu->removeAction(mConnectToAction.get());
                mTrayIconMenu->insertMenu(mDisconnectAction.get(), mConnectToMenu.get());
            }
            mConnectToMenu->setEnabled(true);
            //_ct_menu->setIcon(QIcon(":/icons-tm/connect-red.png"));

            if (!Setting::Instance()->IsShowNodes()) {
                qDebug() << "showNodes is off, so using hubs of size " << hubs.size();
                for (size_t k = 0; k < hubs.size(); ++k) {
                    AServer sr = am->GetSrv(hubs[k]);
                    createMenuItem(mConnectToMenu.get(), sr.name, hubs[k]);//am->ServerIdFromHubId(k));
                }
            } else {
                const std::vector<std::pair<bool, int> > & L0 = am->GetLvl0();
                qDebug() << "showNodes is on, so using servers level 0 size is " << L0.size();
                for (size_t k = 0; k < L0.size(); ++k) {
                    if (L0[k].first) {
                        // hub - add submenu
                        int idhub = L0[k].second;
                        AServer h = am->GetHub(idhub);
                        QMenu * m = new QMenu(h.name);
                        mHubMenus.push_back(m);

                        // add into it all individual servers
                        const std::vector<int> & L1 = am->GetLvl1(idhub);
                        for (size_t k = 0; k < L1.size(); ++k) {
                            int idsrv = L1[k];
                            if (idsrv > -1) {
                                AServer se = am->GetSrv(idsrv);
                                createMenuItem(m, se.name, idsrv);
                            }
                        }
                        mConnectToMenu->addMenu(m);
                    } else {	// just a server without hub
                        int idsrv = L0[k].second;
                        AServer se = am->GetSrv(idsrv);
                        createMenuItem(mConnectToMenu.get(), se.name, idsrv);
                    }
                }

            }
        }
    }
}

void TrayIconManager::clearConnectToMenu()
{
    for (size_t k = 0; k < mHubMenus.size(); ++k)
        delete mHubMenus.at(k);
    mHubMenus.clear();

    // destroy menu items
    if (mConnectToMenu.get())
        if (!mConnectToMenu->isEmpty())
            mConnectToMenu->clear();			// delete actions
}

void TrayIconManager::statusConnecting()
{
    updateActionsEnabled(true);
    updateStateIcon(OpenvpnManager::ovsConnecting);
}

void TrayIconManager::statusConnected()
{
    updateStateIcon(OpenvpnManager::ovsConnected);
    mJumpAction->setEnabled(true);			//_ac_Jump->setIcon(QIcon(":/icons-tm/jump-red.png"));
    mSwitchCountryAction->setEnabled(true);	//_ac_SwitchCountry->setIcon(QIcon(":/icons-tm/country-red.png"));
}

void TrayIconManager::statusDisconnected()
{
    updateActionsEnabled(false);
    updateStateIcon(OpenvpnManager::ovsDisconnected);
}

static void s_set_enabled(QAction * ac, bool enabled, const char * icon_word)
{
    if (!ac)
        return;
    ac->setEnabled(enabled);
//	QString fn = ":/icons-tm/";
//	fn += icon_word;
//	fn += (enabled ? "-red.png" : "-grey.png");
//	ac->setIcon(QIcon(fn));
}
void TrayIconManager::updateActionsEnabled(bool connecting)
{
    QAction * ar[] = {mConnectAction.get(), mJumpAction.get()};
    static const char * words[] = {"connect", "jump"};
    for (size_t k = 0; k < sizeof(words) / sizeof(words[0]); ++k)
        s_set_enabled(ar[k], !connecting, words[k]);

    static const char * country = "country";
    static const char * disconn = "disconnect";
    static const char * conn = "connect";
    if (AuthManager::Instance()->loggedIn()) {
        s_set_enabled(mSwitchCountryAction.get(), !connecting, country);
        s_set_enabled(mDisconnectAction.get(), connecting, disconn);
        s_set_enabled(mConnectToAction.get(), !connecting, conn);

    } else {
        s_set_enabled(mSwitchCountryAction.get(), false, country);
        s_set_enabled(mDisconnectAction.get(), false, disconn);
        s_set_enabled(mConnectToAction.get(), false, conn);
    }
}
