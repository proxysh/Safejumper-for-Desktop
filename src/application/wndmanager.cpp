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

#include "authmanager.h"
#include "log.h"
#include "trayiconmanager.h"
#include "vpnservicemanager.h"
#include "mainwindow.h"

WndManager::WndManager()
{
    connect(TrayIconManager::instance(), &TrayIconManager::bugTriggered,
            this, &WndManager::showFeedback);
    connect(VPNServiceManager::instance(), &VPNServiceManager::stateChanged,
            this, &WndManager::stateChanged);
//    connect(VPNServiceManager::instance(), &VPNServiceManager::error,
//            this, &WndManager::showErrorMessage);
    connect(VPNServiceManager::instance(), &VPNServiceManager::timedOut,
            this, &WndManager::showPortDialog);
}

WndManager::~WndManager()
{
//    if (_DlgPort != NULL) {
//        if (_DlgPort->isVisible())
//            ClosePortDlg();
//        //_DlgPort->deleteLater();
//    }
}

void WndManager::ToPrimary()
{
//    if (AuthManager::instance()->loggedIn())
//        ToConnect();
//    else
//        ToLogin();
}

QWidget * WndManager::Primary()
{
//    if (AuthManager::instance()->loggedIn())
//        return MainWindow::instance();
//    else
//        return LoginWindow::Instance();
    return nullptr;
}

void WndManager::ToLogin()
{
//    QWidget * from = ScrVisible();
//    if (LoginWindow::Instance() != from) {
//        trans(from, LoginWindow::Instance());
//    } else {
//        ToFront(LoginWindow::Instance());		// activate it
//    }
}

void WndManager::ToConnect()
{
//    QWidget * from = ScrVisible();
//    MainWindow::instance()->showConnection();
//    if (MainWindow::instance() != from) {
//        trans(from, MainWindow::instance());
//    } else {
//        ToFront(MainWindow::instance());		// activate it
//    }
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
    if (MainWindow::exists())
        if (MainWindow::instance()->isVisible()) {
//            SaveAndHide(MainWindow::instance());
            ++visible;
        }
}

void WndManager::SaveAndHide(QWidget * from)
{
    SaveCoords(from);
    from->hide();
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
//    int tw = from->width();
    //assert(tw > 0);
    int px = from->x() + (from->width() / 2);
    _x = px - 187;  // 187 int = width of primary / 2
    _y = from->y();
}

void WndManager::ApplyCoords(QWidget * to)
{
//    int tw = to->width();
    //assert(tw > 0);
    int nx = _x + 187 - (to->width() / 2);

//Log::logt(QString().sprintf("ApplyCoords(): moving to (%d,%d)", nx, _y));
    to->move(nx, _y);
}

void WndManager::DoShape(QWidget * d)
{
}

void WndManager::SavePos()
{
    QSettings settings;
    settings.setValue("pos", this->CurrPos());
}

QWidget * WndManager::ScrVisible()
{
    QWidget * w = nullptr;
    int visible = 0;
    if (MainWindow::exists())
        if (MainWindow::instance()->isVisible()) {
//            w = MainWindow::instance();
            ++visible;
        }
    return w;
}

int WndManager::Confirmation(const QString & msg)
{
//    ConfirmationDialog dlg(msg, Primary());
//    return dlg.exec();
    return 0;
}

void WndManager::showPortDialog()
{
    ToPrimary();
//    if (_DlgPort != NULL)
//        _DlgPort->deleteLater();
//    _DlgPort = new Dlg_newnode("Connection failed? Try another node or port.", Primary());
    //_DlgPort->setWindowModality(Qt::ApplicationModal);
//    MainWindow * w = MainWindow::instance();
//    w->connect(_DlgPort, SIGNAL(finished(int)), w, SLOT(portDialogResult(int)));
//    _DlgPort->open();
}

void WndManager::ClosePortDlg()
{
//    if (_DlgPort != NULL) {
//        if (MainWindow::exists()) {
//            MainWindow * w = MainWindow::instance();
//            w->disconnect(_DlgPort, SIGNAL(finished(int)), w, SLOT(portDialogResult(int)));
//        }
//        _DlgPort->close();
//    }
}

bool WndManager::IsCyclePort()
{
//    bool b = true;
//    if (_DlgPort != NULL)
//        b = _DlgPort->IsCyclePort();
//    return b;
    return false;
}

void WndManager::showFeedback()
{
    ToConnect();
    MainWindow::instance()->showFeedback();
}

void WndManager::stateChanged(vpnState state)
{
    switch (state) {
    case vpnStateConnecting:
        // if any form visible
        if (ScrVisible() != nullptr)  // switch to Scr_Connect
            ToPrimary();
        break;
    case vpnStateConnected:
        ClosePortDlg();
        break;
    case vpnStateDisconnected:
        break;
    default:
        break;
    }
}
