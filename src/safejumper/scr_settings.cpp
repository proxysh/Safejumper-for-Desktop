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
#include "scr_map.h"
#include "common.h"
#include "loginwindow.h"
#include "wndmanager.h"
#include "setting.h"
#include "fonthelper.h"
#include "osspecific.h"
#include "log.h"
#include "lvrowdelegateencryption.h"
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
        ui->dd_Encryption->addItem(Setting::EncText(k));
    ui->dd_Encryption->setView(ui->lv_Encryption);
    ui->dd_Encryption->setItemDelegate(new LvRowDelegateEncryption(this));
    _repopulation_inprogress = false;

//	QPoint p0 = _WndStart = pos();
//	WndManager::DoShape(this);
//	QPoint p1 = pos();
//		move(p0);
    qApp->installEventFilter(this);

    {
        QSettings settings;
        ui->cb_AutoConnect->setChecked(settings.value("cb_AutoConnect", false).toBool());
        ui->cb_BlockOnDisconnect->setChecked(settings.value("cb_BlockOnDisconnect", false).toBool());
        ui->cb_DisableIpv6->setChecked(settings.value("cb_DisableIpv6", true).toBool());
        ui->cb_FixDnsLeak->setChecked(settings.value("cb_FixDnsLeak", true).toBool());
        ui->cb_Reconnect->setChecked(settings.value("cb_Reconnect", true).toBool());
        ui->cb_ShowNodes->setChecked(settings.value("cb_ShowNodes", false).toBool());
        ui->cb_Startup->setChecked(settings.value("cb_Startup", true).toBool());
        ui->cb_InsecureWiFi->setChecked(settings.value("cb_InsecureWiFi", false).toBool());

        ui->e_LocalPort->setText(settings.value("e_LocalPort", "9090").toString());
        ui->e_Ports->setText(settings.value("e_Ports", "").toString());
        ui->e_PrimaryDns->setText(settings.value("e_PrimaryDns", "").toString());
        ui->e_SecondaryDns->setText(settings.value("e_SecondaryDns", "").toString());

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
            ui->e_PrimaryDns->setText(Setting::Instance()->DefaultDns1());
        if (ui->e_SecondaryDns->text().isEmpty())
            ui->e_SecondaryDns->setText(Setting::Instance()->DefaultDns2());
    }

    if (ui->cb_Startup->isEnabled())
        OsSpecific::Instance()->SetStartup(ui->cb_Startup->isChecked());
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

        settings.setValue("cb_AutoConnect", ui->cb_AutoConnect->isChecked());
        settings.setValue("cb_BlockOnDisconnect", ui->cb_BlockOnDisconnect->isChecked());
        settings.setValue("cb_DisableIpv6", ui->cb_DisableIpv6->isChecked());
        settings.setValue("cb_FixDnsLeak", ui->cb_FixDnsLeak->isChecked());
        settings.setValue("cb_Reconnect", ui->cb_Reconnect->isChecked());
        settings.setValue("cb_ShowNodes", ui->cb_ShowNodes->isChecked());
        settings.setValue("cb_Startup", ui->cb_Startup->isChecked());
        settings.setValue("cb_InsecureWiFi", ui->cb_InsecureWiFi->isChecked());

        if (IsValidPort(ui->e_LocalPort->text()))
            settings.setValue("e_LocalPort", ui->e_LocalPort->text());
        if (IsPortsValid())
            settings.setValue("e_Ports", ui->e_Ports->text());
        SaveDns(ui->e_PrimaryDns, "e_PrimaryDns", settings);
        SaveDns(ui->e_SecondaryDns, "e_SecondaryDns", settings);

        settings.setValue("dd_Encryption", ui->dd_Encryption->currentIndex());
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
    SaveCb("b_BlockOnDisconnect", v);
}

void Scr_Settings::Toggle_cb_BlockOnDisconnect(bool v)
{
    bool checked = ui->b_BlockOnDisconnect_Line2->isChecked();
    ui->cb_BlockOnDisconnect->setChecked(checked);
    SaveCb("b_BlockOnDisconnect", v);
}

void Scr_Settings::Toggle_cb_Startup(bool v)
{
    SaveCb("cb_Startup", v);
    OsSpecific::Instance()->SetStartup(v);
}

void Scr_Settings::Toggle_cb_AutoConnect(bool v)
{
    SaveCb("cb_AutoConnect", v);
    // TODO: -1 not implemented
}

void Scr_Settings::Toggle_cb_Reconnect(bool v)
{
    SaveCb("cb_Reconnect", v);
    // TODO: -1 not implemented
}

void Scr_Settings::Toggle_cb_InsecureWiFi(bool v)
{
    SaveCb("cb_InsecureWiFi", v);
    if (v)
        LoginWindow::Instance()->StartWifiWatcher();
    else
        LoginWindow::Instance()->StopWifiWatcher();
}

void Scr_Settings::Toggle_cb_ShowNodes(bool v)
{
    Setting::Instance()->ToggleShowNodes(v);
}

void Scr_Settings::Toggle_cb_DisableIpv6(bool v)
{
    SaveCb("cb_DisableIpv6", v);
    try {
        OsSpecific::Instance()->SetIPv6(!v);
    } catch(std::exception & ex) {
        log::logt(ex.what());
    }
}

void Scr_Settings::Toggle_cb_FixDnsLeak(bool v)
{
    SaveCb("cb_FixDnsLeak", v);
    if (v) {
        ui->e_PrimaryDns->setText(Setting::Instance()->DefaultDns1());
        ui->e_SecondaryDns->setText(Setting::Instance()->DefaultDns2());
    } else {	// 2) Please make sure that when we uncheck fix DNS leak, the primary and secondary DNS fields are set to blank https://github.com/proxysh/Safejumper-Desktop/issues/36
        ui->e_PrimaryDns->setText("");			// TODO: -1 force saved settings cleanup
        ui->e_SecondaryDns->setText("");
    }
}

bool Scr_Settings::Is_cb_BlockOnDisconnect()
{
    return ui->cb_BlockOnDisconnect->isChecked();
}

bool Scr_Settings::Is_cb_Startup()
{
    return ui->cb_Startup->isChecked();
}

bool Scr_Settings::Is_cb_AutoConnect()
{
    return ui->cb_AutoConnect->isChecked();
}

bool Scr_Settings::Is_cb_Reconnect()
{
    return ui->cb_Reconnect->isChecked();
}

bool Scr_Settings::Is_cb_InsecureWiFi()
{
    return ui->cb_InsecureWiFi->isChecked();
}

bool Scr_Settings::Is_cb_ShowNodes()
{
    return ui->cb_ShowNodes->isChecked();
}

bool Scr_Settings::Is_cb_DisableIpv6()
{
    return ui->cb_DisableIpv6->isChecked();
}

bool Scr_Settings::Is_cb_FixDnsLeak()
{
    return ui->cb_FixDnsLeak->isChecked();
}

int Scr_Settings::Encryption()
{
    return ui->dd_Encryption->currentIndex();
}

void Scr_Settings::Changed_dd_Encryption(int ix)
{
    if (_repopulation_inprogress)
        return;

    if (ix == ENCRYPTION_OBFS_TOR && !OsSpecific::Instance()->IsObfsInstalled()) {
#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
        // Try to install first, then check if it's not installed again.
        OsSpecific::Instance()->InstallObfs();
        if (!OsSpecific::Instance()->IsObfsInstalled()) {
#endif
            ErrorDialog dlg("Obfsproxy is not compatible with your OS :(", "Encryption error", this);
            dlg.exec();
#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
        }
#endif
    }

    QSettings settings;
    settings.setValue("dd_Encryption", ix);

    if (Scr_Map::IsExists()) {
        Scr_Map::Instance()->RePopulateProtocols();	// list of protocol/ports should be updated to only "OpenVPN TCP 888 (Obfsproxy)".
        Setting::Instance()->LoadProt();
        Scr_Map::Instance()->RePopulateLocations(false); // Repopulate all locations
    }
    if (ConnectionDialog::exists())
        ConnectionDialog::instance()->updateEncoding();
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
        QSettings settings;
        SaveDns(ui->e_PrimaryDns, "e_PrimaryDns", settings);
    }

}

void Scr_Settings::Validate_e_SecondaryDns()
{
    if (Vlidate_e_ip(ui->e_SecondaryDns)) {
        QSettings settings;
        SaveDns(ui->e_SecondaryDns, "e_SecondaryDns", settings);
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
    if (IsPortsValid())
        ui->e_Ports->setStyleSheet(gs_sPortsNorm);
    else
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
    if (IsValidPort(ui->e_LocalPort->text()))
        ui->e_LocalPort->setStyleSheet(gs_sNormStyle);
    else
        ui->e_LocalPort->setStyleSheet(gs_sErrStyle);
}

QString Scr_Settings::Dns1()
{
    return GetDns(ui->e_PrimaryDns);
}

QString Scr_Settings::Dns2()
{
    return GetDns(ui->e_SecondaryDns);
}

QString Scr_Settings::GetDns(QLineEdit * dns)
{
    QString d;
    if (!dns->text().isEmpty())
        if (IsValidIp(dns->text()))
            d = dns->text();
    return d;
}

QString Scr_Settings::LocalPort()
{
    QString p;
    if (IsValidPort(ui->e_LocalPort->text()))
        p = ui->e_LocalPort->text();
    return p;
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
