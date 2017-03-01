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

#include "scr_settings.h"

#include "scr_logs.h"
#include "ui_scr_settings.h"
#include "connectiondialog.h"
#include "mapscreen.h"
#include "common.h"
#include "loginwindow.h"
#include "wndmanager.h"
#include "setting.h"
#include "fonthelper.h"
#include "osspecific.h"
#include "log.h"
#include "encryptiondelegate.h"
#include "errordialog.h"
#include "pathhelper.h"

#include <cstdio>

static bool _repopulation_inprogress = false;

Scr_Settings::Scr_Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Scr_Settings)
    , _moving(false)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());

#ifdef Q_OS_MAC
    ui->e_LocalPort->setAttribute(Qt::WA_MacShowFocusRect, 0);
    ui->e_PrimaryDns->setAttribute(Qt::WA_MacShowFocusRect, 0);
    ui->e_SecondaryDns->setAttribute(Qt::WA_MacShowFocusRect, 0);
    ui->e_Ports->setAttribute(Qt::WA_MacShowFocusRect, 0);
#else
    FontHelper::SetFont(this);
    ui->e_PrimaryDns->setFont(FontHelper::pt(10));
    ui->e_SecondaryDns->setFont(FontHelper::pt(10));
#endif

#ifdef Q_OS_WIN
    setWindowFlags(Qt::Dialog);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif
    // fill Encryption drop down
    _repopulation_inprogress = true;
    ui->dd_Encryption->clear();
    for (int k = 0; k < ENCRYPTION_COUNT; ++k)
        ui->dd_Encryption->addItem(Setting::encryptionName(k));
    ui->dd_Encryption->setView(ui->lv_Encryption);
    ui->dd_Encryption->setItemDelegate(new EncryptionDelegate(this));
    _repopulation_inprogress = false;

//	QPoint p0 = _WndStart = pos();
//	WndManager::DoShape(this);
//	QPoint p1 = pos();
//		move(p0);
    qApp->installEventFilter(this);

    {
        QSettings settings;
        ui->cb_AutoConnect->setChecked(Setting::instance()->autoconnect());
        ui->cb_BlockOnDisconnect->setChecked(Setting::instance()->blockOnDisconnect());
        ui->cb_DisableIpv6->setChecked(Setting::instance()->disableIPv6());
        ui->cb_FixDnsLeak->setChecked(Setting::instance()->fixDns());
        ui->cb_Reconnect->setChecked(Setting::instance()->reconnect());
        ui->cb_ShowNodes->setChecked(Setting::instance()->showNodes());
        ui->cb_Startup->setChecked(Setting::instance()->startup());
        ui->cb_InsecureWiFi->setChecked(Setting::instance()->detectInsecureWifi());

        ui->e_LocalPort->setText(Setting::instance()->localPort());
        ui->e_Ports->setText(Setting::instance()->forwardPortsString());
        ui->e_PrimaryDns->setText(Setting::instance()->dns1());
        ui->e_SecondaryDns->setText(Setting::instance()->dns2());

        ui->dd_Encryption->setCurrentIndex(settings.value("dd_Encryption", 0).toInt());
    }

    // OS-specific not implemented features
#ifdef Q_OS_LINUX
    //ui->cb_InsecureWiFi->setEnabled(false);
    //ui->cb_BlockOnDisconnect->setEnabled(false);
#endif

    // disable non-implemented
    //const char * st = "QPushButton\n{\n	border:0px;\n	color: #b1b1b1;\nborder-image: url(:/imgs/cb-row-norm.png);\n\nimage-position: left;\ntext-align: left;\n}\nQPushButton:checked\n{\ncolor: #3c3c3c;\nborder-image: url(:/imgs/cb-row-checked.png);\n}\nQPushButton:!enabled\n{\n	color: #f0f0f0;\n}";
    if (!ui->cb_AutoConnect->isEnabled()) ui->cb_AutoConnect->setChecked(false);
    if (!ui->cb_BlockOnDisconnect->isEnabled()) ui->cb_BlockOnDisconnect->setChecked(false);
    if (!ui->cb_DisableIpv6->isEnabled()) ui->cb_DisableIpv6->setChecked(false);
    if (!ui->cb_FixDnsLeak->isEnabled()) ui->cb_FixDnsLeak->setChecked(false);
    if (!ui->cb_Reconnect->isEnabled()) ui->cb_Reconnect->setChecked(false);
    if (!ui->cb_ShowNodes->isEnabled()) ui->cb_ShowNodes->setChecked(false);
    if (!ui->cb_Startup->isEnabled()) ui->cb_Startup->setChecked(false);
    if (!ui->cb_InsecureWiFi->isEnabled()) ui->cb_InsecureWiFi->setChecked(false);


    if (ui->cb_FixDnsLeak->isChecked()) {
        if (ui->e_PrimaryDns->text().isEmpty())
            ui->e_PrimaryDns->setText(Setting::instance()->defaultDNS1());
        if (ui->e_SecondaryDns->text().isEmpty())
            ui->e_SecondaryDns->setText(Setting::instance()->defaultDNS2());
    }
}

void Scr_Settings::closeEvent(QCloseEvent * event)
{
    event->ignore();
    WndManager::Instance()->HideThis(this);
}

Scr_Settings::~Scr_Settings()
{
    {
        QSettings settings;
        if (this->isVisible())
            WndManager::Instance()->HideThis(this);
    }
    delete ui;
}

void Scr_Settings::SaveDns(QLineEdit * dns, const char * name, QSettings & settings)
{
    bool er = false;
    if (!dns->text().isEmpty())
        er = !IsValidIp(dns->text());
    if (!er)
        settings.setValue(name, dns->text());
}

std::auto_ptr<Scr_Settings> Scr_Settings::_inst;
Scr_Settings * Scr_Settings::Instance()
{
    if (!_inst.get())
        _inst.reset(new Scr_Settings());
    return _inst.get();
}

void Scr_Settings::ToScr_Connect()
{
    WndManager::Instance()->ToPrimary();
}

void Scr_Settings::ToScr_Logs()
{
    QString path = PathHelper::Instance()->openvpnLogFilename();
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) {
        QTextStream in(&f);
        while (!in.atEnd()) {
            QString l = in.readLine();
            log::logt(l);
        }
        f.close();
    }

    WndManager::Instance()->ToLogs();
}

void Scr_Settings::Clicked_Update()
{
    // https://proxy.sh/version_osx.xml
    launchUpdateUrl();
}

void Scr_Settings::Toggle_BlockOnDisconnect_Line2(bool v)
{
    bool checked = ui->cb_BlockOnDisconnect->isChecked();
    ui->b_BlockOnDisconnect_Line2->setChecked(checked);
    Setting::instance()->setBlockOnDisconnect(v);
}

void Scr_Settings::Toggle_cb_BlockOnDisconnect(bool v)
{
    bool checked = ui->b_BlockOnDisconnect_Line2->isChecked();
    ui->cb_BlockOnDisconnect->setChecked(checked);
    Setting::instance()->setBlockOnDisconnect(v);
}

void Scr_Settings::Toggle_cb_Startup(bool v)
{
    Setting::instance()->setStartup(v);
}

void Scr_Settings::Toggle_cb_AutoConnect(bool v)
{
    Setting::instance()->setAutoconnect(v);
}

void Scr_Settings::Toggle_cb_Reconnect(bool v)
{
    Setting::instance()->setReconnect(v);
}

void Scr_Settings::Toggle_cb_InsecureWiFi(bool v)
{
    Setting::instance()->setDetectInsecureWifi(v);
}

void Scr_Settings::Toggle_cb_ShowNodes(bool v)
{
    Setting::instance()->setShowNodes(v);
}

void Scr_Settings::Toggle_cb_DisableIpv6(bool v)
{
    Setting::instance()->setDisableIPv6(v);
}

void Scr_Settings::Toggle_cb_FixDnsLeak(bool v)
{
    Setting::instance()->setFixDns(v);
    if (v) {
        ui->e_PrimaryDns->setText(Setting::instance()->defaultDNS1());
        ui->e_SecondaryDns->setText(Setting::instance()->defaultDNS2());
    } else {	// 2) Please make sure that when we uncheck fix DNS leak, the primary and secondary DNS fields are set to blank https://github.com/proxysh/Safejumper-Desktop/issues/36
        ui->e_PrimaryDns->setText("");			// TODO: -1 force saved settings cleanup
        ui->e_SecondaryDns->setText("");
    }
}

void Scr_Settings::Changed_dd_Encryption(int ix)
{
    if (_repopulation_inprogress)
        return;

    if ((ix == ENCRYPTION_TOR_OBFS2
         || ix == ENCRYPTION_TOR_OBFS3
         || ix == ENCRYPTION_TOR_SCRAMBLESUIT
         ) && !OsSpecific::instance()->obfsproxyInstalled()) {
#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
        // Try to install first, then check if it's not installed again.
        OsSpecific::instance()->installObfsproxy();
        if (!OsSpecific::instance()->obfsproxyInstalled()) {
#endif
            ErrorDialog dlg("Obfsproxy is not compatible with your OS :(", "Encryption error", this);
            dlg.exec();
#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
        }
#endif
    }

    Setting::instance()->setEncryption(ix);
}

static const char * gs_sErrStyle =
    "QLineEdit\n{\npadding: 2px 10px 2px 10px;\nborder: 0 transparent #b8d4e9;\ncolor: #444444;\nborder-image: url(:/imgs/e-ip-error.png);\n}\n"
    "QLineEdit:focus\n{\nborder-image: url(:/imgs/e-ip-error.png);\n}"
    ;
static const char * gs_sNormStyle =
    "QLineEdit\n{\npadding: 2px 10px 2px 10px;\nborder: 0 transparent #b8d4e9;\ncolor: #444444;\nborder-image: url(:/imgs/e-ip-inactive.png);\n}\n"
    "QLineEdit:focus\n{\nborder-image: url(:/imgs/e-ip-active.png);\n}"
    ;

void Scr_Settings::Validate_e_Dns()
{
    if (Vlidate_e_ip(ui->e_PrimaryDns)) {
        Setting::instance()->setDNS1(ui->e_PrimaryDns->text());
    }

}

void Scr_Settings::Validate_e_SecondaryDns()
{
    if (Vlidate_e_ip(ui->e_SecondaryDns)) {
        Setting::instance()->setDNS2(ui->e_SecondaryDns->text());
    }
}

bool Scr_Settings::Vlidate_e_ip(QLineEdit * eb)
{
    bool error = false;
    if (!eb->text().isEmpty())
        error = !IsValidIp(eb->text());
    if (error)
        eb->setStyleSheet(gs_sErrStyle);
    else
        eb->setStyleSheet(gs_sNormStyle);

    return error;
}

static const char * gs_sPortsNorm =
    "QLineEdit\n{\nborder-radius: 3px;\npadding: 2px 10px 2px 10px;\nborder: 0 transparent #b8d4e9;\ncolor: #444444;\n\nborder-image: url(:/imgs/e-inactive-241.png);\n}\n"
    "QLineEdit:focus\n{\nborder-image: url(:/imgs/e-active-241.png);\n}"
    ;
static const char * gs_sPortsErr =
    "QLineEdit\n{\nborder-radius: 3px;\npadding: 2px 10px 2px 10px;\nborder: 0 transparent #b8d4e9;\ncolor: #444444;\n\nborder-image: url(:/imgs/e-error-241.png);\n}\n"
    "QLineEdit:focus\n{\nborder-image: url(:/imgs/e-active-241.png);\n}"
    ;
void Scr_Settings::Validate_e_Ports()
{
    if (IsPortsValid()) {
        ui->e_Ports->setStyleSheet(gs_sPortsNorm);
        Setting::instance()->setForwardPorts(ui->e_Ports->text());
    } else
        ui->e_Ports->setStyleSheet(gs_sPortsErr);
}

bool Scr_Settings::IsPortsValid(USet * out_ports)	// = NULL)
{
    bool valid = true;
    if (out_ports != NULL)
        out_ports->clear();
    bool tmp;
    int count = 0;
    if (!ui->e_Ports->text().trimmed().isEmpty()) {
        QStringList pp = ui->e_Ports->text().split(",");
        for (int k = 0; k < pp.length(); ++k) {
            if (!IsValidPort(pp[k])) {
                valid = false;
                if (out_ports != NULL)
                    out_ports->clear();
                break;
            } else {
                if (out_ports != NULL)
                    out_ports->insert(pp[k].toInt(&tmp));
                ++count;
            }
        }
    }

    if (valid)
        if (count > PORT_FORWARD_MAX)
            valid = false;

    return valid;
}

USet Scr_Settings::Ports()
{
    USet pp;
    if (!IsPortsValid(&pp))
        pp.clear();
    return pp;
}

void Scr_Settings::Validate_e_LocalPort()
{
    if (IsValidPort(ui->e_LocalPort->text())) {
        ui->e_LocalPort->setStyleSheet(gs_sNormStyle);
        Setting::instance()->setLocalPort(ui->e_LocalPort->text());
    } else
        ui->e_LocalPort->setStyleSheet(gs_sErrStyle);
}

void Scr_Settings::keyPressEvent(QKeyEvent * e)
{
    if(e->key() != Qt::Key_Escape)
        QDialog::keyPressEvent(e);
}


bool Scr_Settings::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseMove: {
        if (_moving) {
            QPoint d = QCursor::pos() - _CursorStart;
            if (d.x() != 0 || d.y() != 0) {
                QPoint NewAbs = _WndStart + d;
                this->move(NewAbs);
            }
        }
        return false;
    }
    case QEvent::MouseButtonRelease: {
        _moving = false;
//			_WndStart = pos();
        return false;
    }
    default:
        return QDialog::eventFilter(obj, event);
    }
}

void Scr_Settings::Pressed_Head()
{
    _WndStart = this->pos();
    _CursorStart = QCursor::pos();
    _moving = true;
}

void Scr_Settings::Clicked_Min()
{
    WndManager::Instance()->HideThis(this);
}

void Scr_Settings::Clicked_Cross()
{
    LoginWindow::Instance()->quitApplication();
}
