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

#ifndef SETTINGSSCREEN_H
#define SETTINGSSCREEN_H

#include <QDialog>
#include <QtWidgets/QLineEdit>
#include <QCloseEvent>
#include <QSettings>
#include <memory>

#include "common.h"

namespace Ui
{
class SettingsScreen;
}

class SettingsScreen : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsScreen(QWidget *parent = 0);
    ~SettingsScreen();

    static SettingsScreen * instance();
    static bool exists();
    static void cleanup();

    USet Ports();

protected:
    virtual void closeEvent(QCloseEvent * event);
    virtual void keyPressEvent(QKeyEvent * event);

private slots:
    void on_startupCheckBox_toggled(bool v);
    void on_autoConnectCheckBox_toggled(bool v);
    void on_reconnectCheckBox_toggled(bool v);
    void on_insecureWifiCheckBox_toggled(bool v);
    void on_showNodesCheckBox_toggled(bool v);
    void on_killSwitchCheckBox_toggled(bool v);
    void on_killSwitchCheckBox2_toggled(bool v);
    void on_disableIPv6CheckBox_toggled(bool v);
    void on_fixDNSCheckBox_toggled(bool v);

    void on_encryptionComboBox_currentIndexChanged(int ix);

    void Validate_e_Dns();
    void Validate_e_SecondaryDns();
    void Validate_e_Ports();
    void Validate_e_LocalPort();

    void on_backButton_clicked();
    void on_showLogsButton_clicked();
    void on_updateButton_clicked();

    void on_loggingButton_toggled(bool v);
private:
    Ui::SettingsScreen * ui;
    static std::auto_ptr<SettingsScreen> mInstance;
    bool Vlidate_e_ip(QLineEdit * eb);
    bool IsPortsValid(USet * out_ports = NULL);
};

#endif // SETTINGSSCREEN_H
