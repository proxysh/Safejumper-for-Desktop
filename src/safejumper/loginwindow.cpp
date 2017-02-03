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

#include "loginwindow.h"

#include <QMenu>
#include <QFontDatabase>

#include "ui_loginwindow.h"
#include "scr_settings.h"
#include "scr_logs.h"
#include "scr_map.h"
#include "errordialog.h"
#include "confirmationdialog.h"
#include "trayiconmanager.h"

#include "authmanager.h"
#include "wndmanager.h"
#include "common.h"
#include "version.h"
#include "setting.h"

#include "openvpnmanager.h"

#include "osspecific.h"
#include "log.h"
#include "fonthelper.h"

LoginWindow::LoginWindow(QWidget *parent) :
    //QDialog(parent),
    QMainWindow(parent),
    ui(new Ui::LoginWindow)
    , _ConnectAfterLogin(false)
    , _activatedcount(0)
    , _wifi_processing(false)
{
    ui->setupUi(this);

    setWindowTitle(qApp->applicationName());
#ifdef Q_OS_WIN
    setWindowFlags(Qt::Dialog);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif
    setFixedSize(this->size());

    ui->cancelButton->hide();

    if (Setting::instance()->testing()) {
        ui->optionsButton->hide();
    }

#ifndef Q_OS_MAC
    FontHelper::SetFont(this);
    ui->eLogin->setFont(FontHelper::pt(14));
    ui->ePsw->setFont(FontHelper::pt(14));
#endif

    ui->eLogin->setAttribute(Qt::WA_MacShowFocusRect, 0);
    ui->ePsw->setAttribute(Qt::WA_MacShowFocusRect, 0);

    ui->L_Version->setText(QString("v ") + SJ_VERSION + ", build " + QString::number(SJ_BUILD_NUM) );

    {
        QSettings settings;
        if (settings.contains("pos")) {
            QPoint p = settings.value("pos").toPoint();
            WndManager::Instance()->trans(p, this);
        }
//		_WndStart = pos();
        ui->rememberMeButton->setChecked(settings.value("cb_Rememberme", true).toBool());

        if (ui->rememberMeButton->isChecked()) {
            if (settings.contains("eLogin"))
                ui->eLogin->setText(settings.value("eLogin", "").toString());
            if (settings.contains("ePsw"))
                ui->ePsw->setText(settings.value("ePsw", "").toString());
        }
    }

    StatusDisconnected();

    if (!ui->eLogin->text().isEmpty() && ui->ePsw->text().isEmpty())
        ui->ePsw->setFocus();
    else
        ui->eLogin->setFocus();
    TrayIconManager::instance()->disableActionsOnLogout();
    connect(TrayIconManager::instance(), SIGNAL(login()), this, SLOT(on_loginButton_clicked()));
    connect(TrayIconManager::instance(), SIGNAL(logout()), this, SLOT(on_cancelButton_clicked()));
    connect(TrayIconManager::instance(), SIGNAL(quitApplication()), this, SLOT(quitApplication()));

//	WndManager::DoShape(this);

    QTimer::singleShot(210, this, SLOT(Timer_Constructed()));

    connect(AuthManager::instance(), SIGNAL(loginCompleted()), this, SLOT(loggedIn()));
    connect(AuthManager::instance(), SIGNAL(loginError(QString)), this, SLOT(loginError(QString)));
}

void LoginWindow::on_rememberMeButton_toggled()
{
    QSettings settings;
    settings.setValue("cb_Rememberme", ui->rememberMeButton->isChecked());
}

void LoginWindow::Timer_Constructed()
{
#ifndef Q_OS_OSX
    connect(g_pTheApp, SIGNAL(showUp()), this, SLOT(ToScr_Primary()));
#endif

    Scr_Logs * l = Scr_Logs::Instance();

    if (l->IsExists())		// force construction
        if (OpenvpnManager::instance()->openvpnRunning())
            OpenvpnManager::instance()->killRunningOpenvpn();

    AuthManager::instance()->getOldIP();

    if (Setting::instance()->checkForUpdates()) {
        AuthManager::instance()->checkUpdates();
    }

    if (Setting::instance()->autoconnect()) {
        _ConnectAfterLogin = true;
        on_loginButton_clicked();
    }

    if (Setting::instance()->detectInsecureWifi()) {
        StartWifiWatcher();
    }
}

LoginWindow::~LoginWindow()
{
    if (this->isVisible())
        WndManager::Instance()->HideThis(this);
    else
        WndManager::Instance()->CloseAll();
    if (this->isVisible())
        WndManager::Instance()->HideThis(this);
    WndManager::Instance()->SavePos();
    on_rememberMeButton_toggled(); // Save remember me setting
    SaveCreds();

    TrayIconManager::cleanup();
    AuthManager::cleanup();
    OpenvpnManager::cleanup();
    OsSpecific::Cleanup();
    Scr_Logs::Cleanup();
    Scr_Map::Cleanup();
    Scr_Settings::Cleanup();
    Setting::cleanup();
    WndManager::Cleanup();

    delete ui;
}

void LoginWindow::SaveCreds()
{
    QSettings settings;
    if (ui->rememberMeButton->isChecked()) {
        settings.setValue("eLogin", ui->eLogin->text());
        settings.setValue("ePsw", ui->ePsw->text());	// TODO: -0
    }
}

void LoginWindow::closeEvent(QCloseEvent * event)
{
    event->ignore();
    WndManager::Instance()->HideThis(this);
}

std::auto_ptr<LoginWindow> LoginWindow::_inst;
LoginWindow * LoginWindow::Instance()
{
    if (!_inst.get())
        _inst.reset(new LoginWindow());
    return _inst.get();
}

void LoginWindow::quitApplication()
{

// HACK: workaround: without any form visible program exits => force show() primary
//	if (WndManager::Instance()->ScrVisible() != NULL)
    WndManager::Instance()->ToPrimary();
    int res = WndManager::Instance()->Confirmation("Would you like to shut Safejumper down?");
    if (res == QDialog::Accepted) {
        WndManager::Instance()->CloseAll();
        if (OpenvpnManager::exists())
            OpenvpnManager::instance()->stop();
        g_pTheApp->quit();
    }
}

void LoginWindow::Cleanup()
{
    std::auto_ptr<LoginWindow> d(_inst.release());
}

void LoginWindow::ToScr_Primary()
{
    WndManager::Instance()->ToPrimary();
}

void LoginWindow::on_cancelButton_clicked()
{
    _CancelLogin = true;
    AuthManager::instance()->cancel();
}

void LoginWindow::on_loginButton_clicked()
{
    if (!AuthManager::instance()->loggedIn()) {
        if (!ui->eLogin->text().isEmpty()) {
            _CancelLogin = false;
            enableButtons(false);
            AuthManager::instance()->login(ui->eLogin->text(), ui->ePsw->text());
        }
    }
}

void LoginWindow::enableButtons(bool enabled)
{
    ui->loginButton->setVisible(enabled);
    ui->cancelButton->setVisible(!enabled);
    ui->eLogin->setEnabled(enabled);
    ui->ePsw->setEnabled(enabled);
    ui->optionsButton->setEnabled(enabled);
}

//void SjMainWindow::Finished_EccxorName()
//{
//	QString errmsg;
//	bool ok = AuthManager::Instance()->ProcessXml_EccxorName(errmsg);
//	if (!ok)
//		log::logt(errmsg);
//}

void LoginWindow::on_optionsButton_clicked()
{
    WndManager::Instance()->ToSettings();
}

void LoginWindow::StatusConnecting()
{
    TrayIconManager::instance()->updateActionsEnabled(true);
    TrayIconManager::instance()->updateStateIcon(OpenvpnManager::ovsConnecting);
}

void LoginWindow::StatusConnected()
{
    TrayIconManager::instance()->updateStateIcon(OpenvpnManager::ovsConnected);
}

void LoginWindow::StatusDisconnected()
{
    TrayIconManager::instance()->updateActionsEnabled(false);
    TrayIconManager::instance()->updateStateIcon(OpenvpnManager::ovsDisconnected);
}


void LoginWindow::StartWifiWatcher()
{
    if (NULL == _timer_wifi.get()) {
        _wifi_processing = false;
        _timer_wifi.reset(new QTimer());
        connect(_timer_wifi.get(), SIGNAL(timeout()), this, SLOT(Timer_WifiWatcher()));
        _timer_wifi->start(5000);
    }
}

void LoginWindow::StopWifiWatcher()
{
    if (NULL != _timer_wifi.get()) {
        QTimer * t = _timer_wifi.release();
        t->stop();
        t->deleteLater();
    }
}

void LoginWindow::Timer_WifiWatcher()
{
    if (NULL != _timer_wifi.get() 		// if not terminating now
            && !_wifi_processing) {			// and not already in the body below
        _wifi_processing = true;
        bool stopped = false;
        if (!AuthManager::exists()) {
            stopped = true;
        } else {
            if (!OpenvpnManager::exists()) {
                stopped = true;
            } else {
                if (OpenvpnManager::instance()->state() == OpenvpnManager::ovsDisconnected)
                    stopped = true;
            }
        }

        if (stopped) {
            if (!AuthManager::instance()->loggedIn()) {
                if (Setting::instance()->autoconnect()) {	// log in only if checked Auto-connect when app starts
                    if (OsSpecific::Instance()->HasInsecureWifi()) {
                        _ConnectAfterLogin = true;
                        on_loginButton_clicked();
                    }
                }
            } else {
                if (OsSpecific::Instance()->HasInsecureWifi())
                    OpenvpnManager::instance()->start();
            }
        }
        _wifi_processing = false;
    }
}

void LoginWindow::BlockOnDisconnect()
{
    // implementation is the same as in the old Safejumper
    bool doblock = false;
    if (Setting::instance()->blockOnDisconnect()) {
        if (AuthManager::exists()) {
            if (!AuthManager::instance()->loggedIn()) {
                doblock = true;
            } else {
                if (!OpenvpnManager::exists()) {
                    doblock = true;
                } else {
                    if (OpenvpnManager::instance()->state() == OpenvpnManager::ovsDisconnected)
                        doblock = true;
                    // otherwise unblocked and should be unblocked
                }
            }
        }
    }

    if (doblock) {
        OsSpecific::Instance()->NetDown();
    }
}

void LoginWindow::loggedIn()
{
    SaveCreds();

    if (Setting::encryption() == ENCRYPTION_RSA)
        Setting::instance()->loadServer();

    // TODO: -0
//		{
//			SETTINGS_OBJ;
//			int ix = settings.value("dd_Protocol_ix", -1).toInt();
//			if (ix < 0)
//			{
//log::logt("sleeping 3");
//				QThread::sleep(3);		// if first run: wait for pings  - to get adequate jump
//			}
//		}
    WndManager::Instance()->ToPrimary();
    TrayIconManager::instance()->enableButtonsOnLogin();
    TrayIconManager::instance()->constructConnectToMenu();
    if (_ConnectAfterLogin)
        OpenvpnManager::instance()->start();
    enableButtons(true);
    _ConnectAfterLogin = false;
}

void LoginWindow::loginError(QString message)
{
    TrayIconManager::instance()->disableActionsOnLogout();
    if (!_CancelLogin) {
        WndManager::Instance()->ToFront(this);
        log::logt("Login Error " + message);
        ErrorDialog dlg(message, "Login Error", this);
        dlg.exec();
    }
    enableButtons(true);
    _ConnectAfterLogin = false;
}
