#include "wndmanager.h"

#include <cassert>
#include <QBitmap>
#include <QPainter>

#include "scr_settings.h"
#include "loginwindow.h"
#include "scr_map.h"
#include "scr_logs.h"
#include "connectiondialog.h"

#include "authmanager.h"
#include "dlg_error.h"
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
    if (AuthManager::Instance()->loggedIn())
        ToConnect();
    else
        ToLogin();
}

QWidget * WndManager::Primary()
{
    if (AuthManager::Instance()->loggedIn())
        return ConnectionDialog::Instance();
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
    if (ConnectionDialog::Instance() != from) {
        trans(from, ConnectionDialog::Instance());
        ConnectionDialog::Instance()->SetAccName(AuthManager::Instance()->VpnName());
    } else {
        ToFront(ConnectionDialog::Instance());		// activate it
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
    if (AuthManager::Instance()->loggedIn())
        trans(ScrVisible(), Scr_Map::Instance());
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
    if (ConnectionDialog::IsExists())
        if (ConnectionDialog::Instance()->isVisible()) {
            SaveAndHide(ConnectionDialog::Instance());
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
    if (Scr_Map::IsExists())
        if (Scr_Map::Instance()->isVisible()) {
            SaveAndHide(Scr_Map::Instance());
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
    to->move(nx, _y);
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
    if (ConnectionDialog::IsExists())
        if (ConnectionDialog::Instance()->isVisible()) {
            w = ConnectionDialog::Instance();
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
    if (Scr_Map::IsExists())
        if (Scr_Map::Instance()->isVisible()) {
            w = Scr_Map::Instance();
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
    ConnectionDialog::Instance()->StatusConnecting();
    if (Scr_Map::IsExists())
        Scr_Map::Instance()->StatusConnecting();
    LoginWindow::Instance()->StatusConnecting();

    // if any form visible
    QWidget * w = ScrVisible();
    if (NULL != w)  // switch to Scr_Connect
        ToPrimary();
    // disable menu buttons

    // change tray icon
    LoginWindow::Instance()->StatusConnecting();
}

void WndManager::HandleConnected()
{
    ClosePortDlg();
    ConnectionDialog::Instance()->StatusConnected();
    LoginWindow::Instance()->StatusConnected();
}

void WndManager::HandleDisconnected()
{
    ConnectionDialog::Instance()->StatusDisconnected();
    if (Scr_Map::IsExists())
        Scr_Map::Instance()->StatusDisconnected();
    LoginWindow::Instance()->StatusDisconnected();
    // TODO: -0
}

void WndManager::HandleState(const QString & word)
{
    ConnectionDialog::Instance()->StatusConnecting(word);
}

void WndManager::ErrMsg(const QString & msg)
{
    this->ToPrimary();
    Dlg_Error dlg(msg, "Error", this->ScrVisible());
    dlg.exec();
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
    ConnectionDialog * w = ConnectionDialog::Instance();
    w->connect(_DlgPort, SIGNAL(finished(int)), w, SLOT(PortDlgAction(int)));
    _DlgPort->open();
}

void WndManager::ClosePortDlg()
{
    if (_DlgPort != NULL) {
        if (ConnectionDialog::IsExists()) {
            ConnectionDialog * w = ConnectionDialog::Instance();
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
