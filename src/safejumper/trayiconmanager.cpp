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

#include "settingsscreen.h"
#include "scr_logs.h"
#include "mapscreen.h"
#include "errordialog.h"
#include "confirmationdialog.h"

#include "authmanager.h"
#include "wndmanager.h"
#include "common.h"
#include "version.h"
#include "setting.h"

#include "openvpnmanager.h"

#include "osspecific.h"
#include "log.h"

TrayIconManager::TrayIconManager(QWidget *parent)
    :mParent(parent)
{
    createTrayIcon();
    mTrayIcon->show();

    statusDisconnected();

    disableActionsOnLogout();

    connect(Setting::instance(), &Setting::showNodesChanged,
            this, &TrayIconManager::constructConnectToMenu);
    connect(Setting::instance(), &Setting::encryptionChanged,
            this, &TrayIconManager::constructConnectToMenu);
    connect(AuthManager::instance(), &AuthManager::serverListsLoaded,
            this, &TrayIconManager::constructConnectToMenu);
    connect(AuthManager::instance(), &AuthManager::loginCompleted,
            this, &TrayIconManager::enableButtonsOnLogin);
    connect(AuthManager::instance(), &AuthManager::logoutCompleted,
            this, &TrayIconManager::disableActionsOnLogout);

    // On OSX check update the state icon every 3 seconds so we can change
    // from dark to light icons as the theme changes
    // TODO: Get notified by the os when this user setting changes instead
#ifdef Q_OS_DARWIN
    if (mIconTimer.get() == NULL) {
        mIconTimer.reset(new QTimer(this));
        connect(mIconTimer.get(), SIGNAL(timeout()),
                this, SLOT(updateStateIcon()));
        mIconTimer->start(3000);
    }
#endif
}

bool TrayIconManager::exists()
{
    return (mInstance.get() != NULL);
}

void TrayIconManager::disableActionsOnLogout()
{
    mBugAction->setEnabled(false);
    mLogoutAction->setEnabled(false);	//_ac_Logout->setIcon(QIcon(":/icons-tm/close-grey.png"));
    mJumpAction->setEnabled(false);	//_ac_Jump->setIcon(QIcon(":/icons-tm/jump-grey.png"));
    mSwitchCountryAction->setEnabled(false);	//_ac_SwitchCountry->setIcon(QIcon(":/icons-tm/country-grey.png"));
}

void TrayIconManager::enableButtonsOnLogin()
{
    mBugAction->setEnabled(true);
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
    QIcon icon(OsSpecific::instance()->disconnectedIcon());
    mTrayIcon->setIcon(icon);
    connect(mTrayIcon.get(), &QSystemTrayIcon::activated,
            this, &TrayIconManager::actionActivated);
}

void TrayIconManager::createTrayIconMenu()
{
    mTrayIconMenu.reset(new QMenu(mParent));
    createMenuActions();

    if (!Setting::instance()->testing()) {
        mTrayIconMenu->addAction(mConnectAction.get());
        mTrayIconMenu->addAction(mConnectToAction.get());
        mTrayIconMenu->addAction(mDisconnectAction.get());
        mTrayIconMenu->addSeparator();
    }

    mTrayIconMenu->addAction(mStatusAction.get());
    mTrayIconMenu->addSeparator();

    if (!Setting::instance()->testing()) {
        mTrayIconMenu->addAction(mJumpAction.get());
        mTrayIconMenu->addAction(mSwitchCountryAction.get());
        mSwitchCountryAction->setEnabled(false);
        mTrayIconMenu->addSeparator();

        mTrayIconMenu->addAction(mSettingsAction.get());
    }

    mTrayIconMenu->addAction(mLogsAction.get());
    mTrayIconMenu->addSeparator();

    if (!Setting::instance()->testing()) {
        mTrayIconMenu->addAction(mWebManageAction.get());
        mTrayIconMenu->addAction(mSupportAction.get());
        mTrayIconMenu->addAction(mBugAction.get());
        mTrayIconMenu->addAction(mEarnAction.get());
        mTrayIconMenu->addSeparator();
    }

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
    connect(mBugAction.get(), &QAction::triggered,
            this, &TrayIconManager::bugTriggered);

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

void TrayIconManager::actionActivated(QSystemTrayIcon::ActivationReason reason)
{
#ifdef Q_OS_WIN
    if (reason == QSystemTrayIcon::DoubleClick)
        statusTriggered();
    else if (reason == QSystemTrayIcon::Trigger && !mTrayIconMenu->isVisible())
        mTrayIconMenu->exec();
#else
    Q_UNUSED(reason);
#endif
}

void TrayIconManager::updateStateIcon()
{
    OpenvpnManager::OvState st = OpenvpnManager::ovsDisconnected;
    if (OpenvpnManager::exists())
        st = OpenvpnManager::instance()->state();
    updateStateIcon(st);
}

void TrayIconManager::updateStateIcon(OpenvpnManager::OvState st)
{
    QString ic;
    switch (st) {
    case OpenvpnManager::ovsDisconnected:
        ic = OsSpecific::instance()->disconnectedIcon();
        break;
    case OpenvpnManager::ovsConnecting:
        ic = OsSpecific::instance()->connectingIcon();
        break;
    case OpenvpnManager::ovsConnected:
        ic = OsSpecific::instance()->connectedIcon();
        mJumpAction->setEnabled(true);
        mSwitchCountryAction->setEnabled(true);
        break;
    default:
        break;
    }
    QIcon icon(ic);
    mTrayIcon->setIcon(icon);
}

void TrayIconManager::connectTriggered()
{
    if (!AuthManager::instance()->loggedIn()) {
        emit login();
    } else {
        OpenvpnManager::instance()->start();
    }
}

void TrayIconManager::connectToTriggered()
{
    // Get the action from the sender
    QAction *action = qobject_cast<QAction*>(sender());
    if (action != NULL) {
        // Get the action's server id
        if (AuthManager::instance()->loggedIn()) {
            size_t serverId = action->data().toInt();

            Setting::instance()->setServer(serverId);
            OpenvpnManager::instance()->start();
        } else {
            emit login();
        }
    }
}

void TrayIconManager::disconnectTriggered()
{
    OpenvpnManager::instance()->stop();
}

void TrayIconManager::statusTriggered()
{
    WndManager::Instance()->ToPrimary();
}

void TrayIconManager::jumpTriggered()
{
    WndManager::Instance()->ToPrimary();
    AuthManager::instance()->jump();
}

void TrayIconManager::switchCountryTriggered()
{
    WndManager::Instance()->ToMap();
}

void TrayIconManager::settingsTriggered()
{
    WndManager::Instance()->ToSettings();
}

void TrayIconManager::logsTriggered()
{
    WndManager::Instance()->ToLogs();
}

void TrayIconManager::webManagTriggered()
{
    WndManager::Instance()->CloseAll();
    OpenUrl_Panel();
}

void TrayIconManager::supportTriggered()
{
    WndManager::Instance()->CloseAll();
    OpenUrl_Support();
}

void TrayIconManager::earnTriggered()
{
    WndManager::Instance()->CloseAll();
    OpenUrl_Earn();
}

void TrayIconManager::aboutTriggered()
{
    WndManager::Instance()->ToPrimary();
}

void TrayIconManager::closeTriggered()
{
    emit quitApplication();
}

void TrayIconManager::logoutTriggered()
{
    if (OpenvpnManager::exists())
        OpenvpnManager::instance()->stop();
    if (AuthManager::exists())
        AuthManager::instance()->logout();
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
    if (Setting::instance()->testing())
        return;

    if (AuthManager::exists()) {
        AuthManager * am = AuthManager::instance();
        if (am->loggedIn()) {
            clearConnectToMenu();

            if (mConnectToMenu.get() == NULL) {	// one time during entire program run
                mConnectToMenu.reset(mTrayIconMenu->addMenu("Connect to ..."));
                mTrayIconMenu->removeAction(mConnectToAction.get());
                mTrayIconMenu->insertMenu(mDisconnectAction.get(), mConnectToMenu.get());
            }
            if (Setting::instance()->showNodes() || Setting::instance()->encryption() >= ENCRYPTION_ECC) {
                // Use list of servers instead of hubs for ECC and ECC_XOR
                const QList<int> &servers = am->currentEncryptionServers();
                for (int k = 0; k < servers.size(); ++k) {
                    AServer server = am->getServer(servers.at(k));
                    createMenuItem(mConnectToMenu.get(), server.name, servers.at(k));
                }
            } else {
                const QList<int> & hubs = am->currentEncryptionHubs();
                //_ct_menu->setIcon(QIcon(":/icons-tm/connect-red.png"));

                const std::vector<std::pair<bool, int> > & L0 = am->getLevel0();
                for (size_t k = 0; k < L0.size(); ++k) {
                    if (L0[k].first) {
                        // hub - add submenu
                        int idhub = L0[k].second;
                        AServer h = am->getHub(idhub);
                        QMenu * m = new QMenu(h.name);
                        mHubMenus.push_back(m);

                        // add into it all individual servers
                        const std::vector<int> & L1 = am->getLevel1(idhub);
                        for (size_t k = 0; k < L1.size(); ++k) {
                            int idsrv = L1[k];
                            if (idsrv > -1) {
                                AServer se = am->getServer(idsrv);
                                createMenuItem(m, se.name, idsrv);
                            }
                        }
                        mConnectToMenu->addMenu(m);
                    } else {	// just a server without hub
                        int idsrv = L0[k].second;
                        AServer se = am->getServer(idsrv);
                        createMenuItem(mConnectToMenu.get(), se.name, idsrv);
                    }
                }
            }
            mConnectToMenu->setEnabled(true);
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

static void s_set_enabled(QAction * ac, bool enabled, const char * /*icon_word */)
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
    if (AuthManager::instance()->loggedIn()) {
        s_set_enabled(mSwitchCountryAction.get(), !connecting, country);
        s_set_enabled(mDisconnectAction.get(), connecting, disconn);
        s_set_enabled(mConnectToAction.get(), !connecting, conn);

    } else {
        s_set_enabled(mSwitchCountryAction.get(), false, country);
        s_set_enabled(mDisconnectAction.get(), false, disconn);
        s_set_enabled(mConnectToAction.get(), false, conn);
    }
}
