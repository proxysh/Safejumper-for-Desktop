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

#include "wndmanager.h"

#include <cassert>
#include <QBitmap>
#include <QPainter>

#include "mapscreen.h"
#include "scr_settings.h"
#include "loginwindow.h"
#include "scr_logs.h"
#include "testdialog.h"

#ifdef MONITOR_TOOL
#include "scr_table.h"
#include "setting.h"
#endif	// MONITOR_TOOL

#include "authmanager.h"
#include "errordialog.h"
#include "log.h"

WndManager::WndManager()
    : _DlgPort(NULL)
{}

WndManager::~WndManager()
{
    if (_DlgPort != NULL) {
        if (_DlgPort->isVisible())
            ClosePortDlg();
        //_DlgPort->deleteLater();
    }
}

void WndManager::ToPrimary()
{
    if (AuthManager::instance()->loggedIn())
        ToConnect();
    else
        ToLogin();
}

QWidget * WndManager::Primary()
{
    if (AuthManager::instance()->loggedIn())
        return TestDialog::instance();
    else
        return LoginWindow::Instance();
}

void WndManager::ToLogin()
{
    QWidget * from = ScrVisible();
    if (LoginWindow::Instance() != from) {
        trans(from, LoginWindow::Instance());
    } else {
        ToFront(LoginWindow::Instance());		// activate it
    }
}

void WndManager::ToConnect()
{
    QWidget * from = ScrVisible();
    if (TestDialog::instance() != from) {
        trans(from, TestDialog::instance());
        TestDialog::instance()->setAccountName(AuthManager::instance()->VPNName());
    } else {
        ToFront(TestDialog::instance());		// activate it
    }
}

void WndManager::ToSettings()
{
    trans(ScrVisible(), Scr_Settings::Instance());
}

void WndManager::ToLogs()
{
    trans(ScrVisible(), Scr_Logs::Instance());
}

void WndManager::ToMap()
{
    if (AuthManager::instance()->loggedIn())
        trans(ScrVisible(), MapScreen::instance());
    else
        ToLogin();
}

std::auto_ptr<WndManager> WndManager::_inst;
WndManager * WndManager::Instance()
{
    if (!_inst.get())
        _inst.reset(new WndManager());
    return _inst.get();
}

void WndManager::DoTrans(QWidget * to)
{
    ApplyCoords(to);
    ToFront(to);
}

void WndManager::ToFront(QWidget * w)
{
    if (w) {
        w->show();
        w->raise();
        w->activateWindow();
    }
}

void WndManager::ToFront()
{
    ToFront(ScrVisible());
}

void WndManager::trans(QWidget * from, QWidget * to)
{
    if (from) {
        if (from != to)
            SaveAndHide(from);
    } else
        CloseAll();
    DoTrans(to);
}

void WndManager::trans(const QPoint & newpos, QWidget * to)
{
    _x = newpos.x();
    _y = newpos.y();
    DoTrans(to);
}

QPoint WndManager::CurrPos()
{
    return QPoint(_x, _y);
}

void WndManager::CloseAll()
{
    int visible = 0;
    if (TestDialog::exists())
        if (TestDialog::instance()->isVisible()) {
            SaveAndHide(TestDialog::instance());
            ++visible;
        }
    if (LoginWindow::IsExists())
        if (LoginWindow::Instance()->isVisible()) {
            SaveAndHide(LoginWindow::Instance());
            ++visible;
        }
    if (Scr_Logs::IsExists())
        if (Scr_Logs::Instance()->isVisible()) {
            SaveAndHide(Scr_Logs::Instance());
            ++visible;
        }
    if (MapScreen::exists())
        if (MapScreen::instance()->isVisible()) {
            SaveAndHide(MapScreen::instance());
            ++visible;
        }
    if (Scr_Settings::IsExists())
        if (Scr_Settings::Instance()->isVisible()) {
            SaveAndHide(Scr_Settings::Instance());
            ++visible;
        }
    //assert(visible < 2);
}

void WndManager::SaveAndHide(QWidget * from)
{
    SaveCoords(from);
    from->hide();
}

void WndManager::ShowTable()
{
#ifdef MONITOR_TOOL
    Scr_Table::Instance()->show();
#endif	// MONITOR_TOOL
}

void WndManager::HideThis(QWidget * scr)
{
    SaveAndHide(scr);
}

void WndManager::SaveCoords(QWidget * from)
{
    if (!from)
        return;
    if (!from->isVisible())
        return;
    int tw = from->width();
    //assert(tw > 0);
    int px = from->x() + (from->width() / 2);
    _x = px - 187;  // 187 int = width of primary / 2
    _y = from->y();
}

void WndManager::ApplyCoords(QWidget * to)
{
    int tw = to->width();
    //assert(tw > 0);
    int nx = _x + 187 - (to->width() / 2);

//log::logt(QString().sprintf("ApplyCoords(): moving to (%d,%d)", nx, _y));
//    to->move(nx, _y);
}

void WndManager::DoShape(QWidget * d)
{
    d->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
}

void WndManager::SavePos()
{
    QSettings settings;
    settings.setValue("pos", this->CurrPos());
}

QWidget * WndManager::ScrVisible()
{
    QWidget * w = NULL;
    int visible = 0;
    if (TestDialog::exists())
        if (TestDialog::instance()->isVisible()) {
            w = TestDialog::instance();
            ++visible;
        }
    if (LoginWindow::IsExists())
        if (LoginWindow::Instance()->isVisible()) {
            w = LoginWindow::Instance();
            ++visible;
        }
    if (Scr_Logs::IsExists())
        if (Scr_Logs::Instance()->isVisible()) {
            w = Scr_Logs::Instance();
            ++visible;
        }
    if (MapScreen::exists())
        if (MapScreen::instance()->isVisible()) {
            w = MapScreen::instance();
            ++visible;
        }
    if (Scr_Settings::IsExists())
        if (Scr_Settings::Instance()->isVisible()) {
            w = Scr_Settings::Instance();
            ++visible;
        }
    //assert(visible < 2);
    return w;
}

void WndManager::HandleConnecting()
{
    // disable buttons
    // change status label to connecting
    TestDialog::instance()->setStatusConnecting();
    if (MapScreen::exists())
        MapScreen::instance()->statusConnecting();
    LoginWindow::Instance()->StatusConnecting();

    // if any form visible
#ifndef MONITOR_TOOL
    QWidget * w = ScrVisible();
    if (NULL != w)  // switch to Scr_Connect
        ToPrimary();
#endif	// MONITOR_TOOL
    // disable menu buttons

    // change tray icon
    LoginWindow::Instance()->StatusConnecting();
}

void WndManager::HandleConnected()
{
    ClosePortDlg();
    TestDialog::instance()->setStatusConnected();
    LoginWindow::Instance()->StatusConnected();

#ifdef MONITOR_TOOL
    Scr_Table::Instance()->SetStatus(Setting::Encryption(), Setting::Instance()->ServerID(), Setting::Instance()->CurrProto(), snsConnected);
#endif	// MONITOR_TOOL
}

void WndManager::HandleDisconnected()
{
    TestDialog::instance()->setStatusDisconnected();
    if (MapScreen::exists())
        MapScreen::instance()->statusDisconnected();
    LoginWindow::Instance()->StatusDisconnected();
    // TODO: -0

#ifdef MONITOR_TOOL
    if (!Ctr_Openvpn::Instance()->IsInNextPort()) {
        if (Ctr_Openvpn::Instance()->IsErr())
            Scr_Table::Instance()->SetStatus(Setting::Encryption(), Setting::Instance()->ServerID(), Setting::Instance()->CurrProto(), Ctr_Openvpn::Instance()->ErrMsg());
        else
            Scr_Table::Instance()->SetStatus(Setting::Encryption(), Setting::Instance()->ServerID(), Setting::Instance()->CurrProto(), snsDown);
    }
#endif	// MONITOR_TOOL

}

void WndManager::HandleState(const QString & word)
{
    TestDialog::instance()->setStatusConnecting(word);
}

void WndManager::ErrMsg(const QString & msg)
{
    TestDialog::instance()->setError(msg);
}

int WndManager::Confirmation(const QString & msg)
{
    ConfirmationDialog dlg(msg, Primary());
    return dlg.exec();
}

void WndManager::ShowPortDlg()
{
    ToPrimary();
    if (_DlgPort != NULL)
        _DlgPort->deleteLater();
    _DlgPort = new Dlg_newnode("Connection failed? Try another node or port.", Primary());
    //_DlgPort->setWindowModality(Qt::ApplicationModal);
    TestDialog * w = TestDialog::instance();
    w->connect(_DlgPort, SIGNAL(finished(int)), w, SLOT(PortDlgAction(int)));
    _DlgPort->open();
}

void WndManager::ClosePortDlg()
{
    if (_DlgPort != NULL) {
        if (TestDialog::exists()) {
            TestDialog * w = TestDialog::instance();
            w->disconnect(_DlgPort, SIGNAL(finished(int)), w, SLOT(PortDlgAction(int)));
        }
        _DlgPort->close();
    }
}

bool WndManager::IsCyclePort()
{
    bool b = true;
    if (_DlgPort != NULL)
        b = _DlgPort->IsCyclePort();
    return b;
}
