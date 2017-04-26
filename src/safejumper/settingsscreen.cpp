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

#include "settingsscreen.h"

#include "scr_logs.h"
#include "ui_settingsscreen.h"
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

SettingsScreen::SettingsScreen(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsScreen)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());

#ifdef Q_OS_DARWIN
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
    ui->encryptionComboBox->clear();
    for (int k = 0; k < ENCRYPTION_COUNT; ++k)
        ui->encryptionComboBox->addItem(Setting::encryptionName(k));
    ui->encryptionComboBox->setItemDelegate(new EncryptionDelegate(this));
    _repopulation_inprogress = false;

    ui->autoConnectCheckBox->setChecked(Setting::instance()->autoconnect());
    ui->killSwitchCheckBox->setChecked(Setting::instance()->blockOnDisconnect());
    ui->disableIPv6CheckBox->setChecked(Setting::instance()->disableIPv6());
    ui->fixDNSCheckBox->setChecked(Setting::instance()->fixDns());
    ui->reconnectCheckBox->setChecked(Setting::instance()->reconnect());
    ui->showNodesCheckBox->setChecked(Setting::instance()->showNodes());
    ui->startupCheckBox->setChecked(Setting::instance()->startup());
    ui->insecureWifiCheckBox->setChecked(Setting::instance()->detectInsecureWifi());

    ui->e_LocalPort->setText(Setting::instance()->localPort());
    ui->e_Ports->setText(Setting::instance()->forwardPortsString());
    ui->e_PrimaryDns->setText(Setting::instance()->dns1());
    ui->e_SecondaryDns->setText(Setting::instance()->dns2());
    ui->loggingButton->setChecked(Setting::instance()->logging());

    ui->encryptionComboBox->setCurrentIndex(Setting::instance()->encryption());

    // OS-specific not implemented features
#ifdef Q_OS_LINUX
    //ui->cb_InsecureWiFi->setEnabled(false);
    //ui->cb_BlockOnDisconnect->setEnabled(false);
#endif

    // disable non-implemented
    if (!ui->autoConnectCheckBox->isEnabled()) ui->autoConnectCheckBox->setChecked(false);
    if (!ui->killSwitchCheckBox->isEnabled()) ui->killSwitchCheckBox->setChecked(false);
    if (!ui->disableIPv6CheckBox->isEnabled()) ui->disableIPv6CheckBox->setChecked(false);
    if (!ui->fixDNSCheckBox->isEnabled()) ui->fixDNSCheckBox->setChecked(false);
    if (!ui->reconnectCheckBox->isEnabled()) ui->reconnectCheckBox->setChecked(false);
    if (!ui->showNodesCheckBox->isEnabled()) ui->showNodesCheckBox->setChecked(false);
    if (!ui->startupCheckBox->isEnabled()) ui->startupCheckBox->setChecked(false);
    if (!ui->insecureWifiCheckBox->isEnabled()) ui->insecureWifiCheckBox->setChecked(false);


    if (ui->fixDNSCheckBox->isChecked()) {
        if (ui->e_PrimaryDns->text().isEmpty())
            ui->e_PrimaryDns->setText(Setting::instance()->defaultDNS1());
        if (ui->e_SecondaryDns->text().isEmpty())
            ui->e_SecondaryDns->setText(Setting::instance()->defaultDNS2());
    }
}

void SettingsScreen::closeEvent(QCloseEvent * event)
{
    event->ignore();
    WndManager::Instance()->HideThis(this);
}

SettingsScreen::~SettingsScreen()
{
    {
        if (this->isVisible())
            WndManager::Instance()->HideThis(this);
    }
    delete ui;
}

std::auto_ptr<SettingsScreen> SettingsScreen::mInstance;
SettingsScreen * SettingsScreen::instance()
{
    if (!mInstance.get())
        mInstance.reset(new SettingsScreen());
    return mInstance.get();
}

bool SettingsScreen::exists()
{
    return (mInstance.get() != NULL);
}

void SettingsScreen::cleanup()
{
    if (mInstance.get() != NULL) delete mInstance.release();
}

void SettingsScreen::on_backButton_clicked()
{
    WndManager::Instance()->ToPrimary();
}

void SettingsScreen::on_showLogsButton_clicked()
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

void SettingsScreen::on_updateButton_clicked()
{
    // https://proxy.sh/version_osx.xml
    launchUpdateUrl();
}

void SettingsScreen::on_loggingButton_toggled(bool v)
{
    Setting::instance()->setLogging(v);
}

void SettingsScreen::on_killSwitchCheckBox_toggled(bool v)
{
    Setting::instance()->setBlockOnDisconnect(v);
}

void SettingsScreen::on_startupCheckBox_toggled(bool v)
{
    Setting::instance()->setStartup(v);
}

void SettingsScreen::on_autoConnectCheckBox_toggled(bool v)
{
    Setting::instance()->setAutoconnect(v);
}

void SettingsScreen::on_reconnectCheckBox_toggled(bool v)
{
    Setting::instance()->setReconnect(v);
}

void SettingsScreen::on_insecureWifiCheckBox_toggled(bool v)
{
    Setting::instance()->setDetectInsecureWifi(v);
}

void SettingsScreen::on_showNodesCheckBox_toggled(bool v)
{
    Setting::instance()->setShowNodes(v);
}

void SettingsScreen::on_disableIPv6CheckBox_toggled(bool v)
{
    Setting::instance()->setDisableIPv6(v);
}

void SettingsScreen::on_fixDNSCheckBox_toggled(bool v)
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

void SettingsScreen::on_encryptionComboBox_currentIndexChanged(int ix)
{
    if (_repopulation_inprogress)
        return;

    if ((ix == ENCRYPTION_TOR_OBFS2
            || ix == ENCRYPTION_TOR_OBFS3
            || ix == ENCRYPTION_TOR_SCRAMBLESUIT
        ) && !OsSpecific::instance()->obfsproxyInstalled()) {
#if defined(Q_OS_DARWIN) || defined(Q_OS_LINUX)
        // Try to install first, then check if it's not installed again.
        OsSpecific::instance()->installObfsproxy();
        if (!OsSpecific::instance()->obfsproxyInstalled()) {
#endif
            ErrorDialog dlg("Obfsproxy is not compatible with your OS :(", "Encryption error", this);
            dlg.exec();
#if defined(Q_OS_DARWIN) || defined(Q_OS_LINUX)
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

void SettingsScreen::Validate_e_Dns()
{
    if (Vlidate_e_ip(ui->e_PrimaryDns)) {
        Setting::instance()->setDNS1(ui->e_PrimaryDns->text());
    }

}

void SettingsScreen::Validate_e_SecondaryDns()
{
    if (Vlidate_e_ip(ui->e_SecondaryDns)) {
        Setting::instance()->setDNS2(ui->e_SecondaryDns->text());
    }
}

bool SettingsScreen::Vlidate_e_ip(QLineEdit * eb)
{
    bool error = false;
    if (!eb->text().isEmpty())
        error = !IsValidIp(eb->text());
    if (error)
        eb->setStyleSheet(gs_sErrStyle);
    else
        eb->setStyleSheet(gs_sNormStyle);

    return !error;
}

static const char * gs_sPortsNorm =
    "QLineEdit\n{\nborder-radius: 3px;\npadding: 2px 10px 2px 10px;\nborder: 0 transparent #b8d4e9;\ncolor: #444444;\n\nborder-image: url(:/imgs/e-inactive-241.png);\n}\n"
    "QLineEdit:focus\n{\nborder-image: url(:/imgs/e-active-241.png);\n}"
    ;
static const char * gs_sPortsErr =
    "QLineEdit\n{\nborder-radius: 3px;\npadding: 2px 10px 2px 10px;\nborder: 0 transparent #b8d4e9;\ncolor: #444444;\n\nborder-image: url(:/imgs/e-error-241.png);\n}\n"
    "QLineEdit:focus\n{\nborder-image: url(:/imgs/e-active-241.png);\n}"
    ;
void SettingsScreen::Validate_e_Ports()
{
    if (IsPortsValid()) {
        ui->e_Ports->setStyleSheet(gs_sPortsNorm);
        Setting::instance()->setForwardPorts(ui->e_Ports->text());
    } else
        ui->e_Ports->setStyleSheet(gs_sPortsErr);
}

bool SettingsScreen::IsPortsValid(USet * out_ports)	// = NULL)
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

USet SettingsScreen::Ports()
{
    USet pp;
    if (!IsPortsValid(&pp))
        pp.clear();
    return pp;
}

void SettingsScreen::Validate_e_LocalPort()
{
    if (IsValidPort(ui->e_LocalPort->text())) {
        ui->e_LocalPort->setStyleSheet(gs_sNormStyle);
        Setting::instance()->setLocalPort(ui->e_LocalPort->text());
    } else
        ui->e_LocalPort->setStyleSheet(gs_sErrStyle);
}

void SettingsScreen::keyPressEvent(QKeyEvent * e)
{
    if(e->key() != Qt::Key_Escape)
        QDialog::keyPressEvent(e);
}
