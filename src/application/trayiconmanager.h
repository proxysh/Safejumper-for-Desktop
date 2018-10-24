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

#ifndef TRAYICONMANAGER_H
#define TRAYICONMANAGER_H

#include <QSystemTrayIcon>

#include "common.h"

#include <memory>

class QAction;

// This class manages the system tray icon and it's menu.
class TrayIconManager : public QObject
{
    Q_OBJECT
public:
    static TrayIconManager * instance();
    static bool exists();
    static void cleanup();

    ~TrayIconManager();

    void statusConnecting();
    void statusConnected();
    void statusDisconnected();

    void updateActionsEnabled(bool connecting);

signals:
    void login();
    void logout();
    void connectToServer(int serverId);
    void quitApplication();

    void bugTriggered();

public slots:
    void loggedInChanged();

    void constructConnectToMenu();

    void connectTriggered();
    void connectToTriggered();
    void disconnectTriggered();
    void statusTriggered();
    void jumpTriggered();
    void switchCountryTriggered();
    void settingsTriggered();
    void logsTriggered();
    void webManagTriggered();
    void supportTriggered();
    void earnTriggered();
    void aboutTriggered();
    void logoutTriggered();
    void closeTriggered();

private slots:
    void disableActionsOnLogout();
    void enableButtonsOnLogin();

    void refreshStateIcon();

    void actionActivated(QSystemTrayIcon::ActivationReason reason);

    void stateChanged(vpnState st);

private:
    explicit TrayIconManager(QWidget *parent = nullptr);

    static std::auto_ptr<TrayIconManager> mInstance;

    void createTrayIconMenu();
    void createMenuActions();
    void createTrayIcon();
    void clearConnectToMenu();
    void createMenuItem(QMenu * m, const QString & name, size_t srv);

#ifdef Q_OS_DARWIN
    bool isDark() const;
#endif

    const QString disconnectedIcon() const;
    const QString connectingIcon() const;
    const QString connectedIcon() const;

    const QString disconnectedSelectedIcon() const;
    const QString connectingSelectedIcon() const;
    const QString connectedSelectedIcon() const;

    std::auto_ptr<QSystemTrayIcon> mTrayIcon;
    std::auto_ptr<QMenu> mTrayIconMenu;

    std::auto_ptr<QAction> mConnectAction;
    std::auto_ptr<QAction> mConnectToAction;
    std::auto_ptr<QAction> mDisconnectAction;
    std::auto_ptr<QAction> mStatusAction;
    std::auto_ptr<QAction> mJumpAction;
    std::auto_ptr<QAction> mSwitchCountryAction;
    std::auto_ptr<QAction> mSettingsAction;
    std::auto_ptr<QAction> mLogsAction;
    std::auto_ptr<QAction> mWebManageAction;
    std::auto_ptr<QAction> mSupportAction;
    std::auto_ptr<QAction> mBugAction;
    std::auto_ptr<QAction> mEarnAction;
    std::auto_ptr<QAction> mAboutAction;
    std::auto_ptr<QAction> mLogoutAction;
    std::auto_ptr<QAction> mCloseAction;

#ifdef Q_OS_DARWIN
    std::auto_ptr<QTimer> mIconTimer;
#endif

    std::auto_ptr<QMenu> mConnectToMenu;
    std::vector<QMenu *> mHubMenus;

    QWidget *mParent;
};

#endif // SJMAINWINDOW_H
