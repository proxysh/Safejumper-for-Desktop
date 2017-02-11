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

#ifndef SCR_SETTINGS_H
#define SCR_SETTINGS_H

#include <QDialog>
#include <QtWidgets/QLineEdit>
#include <QCloseEvent>
#include <QSettings>
#include <memory>

#include "common.h"

namespace Ui
{
class Scr_Settings;
}

class Scr_Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Scr_Settings(QWidget *parent = 0);
    ~Scr_Settings();

    static Scr_Settings * Instance();
    static bool IsExists()
    {
        return (_inst.get() != NULL);
    }
    static void Cleanup()
    {
        if (_inst.get() != NULL) delete _inst.release();
    }

    USet Ports();

private:
    Ui::Scr_Settings * ui;
    static std::auto_ptr<Scr_Settings> _inst;
    bool Vlidate_e_ip(QLineEdit * eb);
    bool IsPortsValid(USet * out_ports = NULL);
    void SaveDns(QLineEdit * dns, const char * name, QSettings & settings);
    bool _moving;
    QPoint _WndStart;
    QPoint _CursorStart;



protected:
    virtual void closeEvent(QCloseEvent * event);
    virtual void keyPressEvent(QKeyEvent * event);
    virtual bool eventFilter(QObject *obj, QEvent *event);

public slots:
    void Pressed_Head();
    void Clicked_Min();
    void Clicked_Cross();

private slots:
    void Toggle_BlockOnDisconnect_Line2(bool v);
    void Toggle_cb_BlockOnDisconnect(bool v);
    void Toggle_cb_Startup(bool v);
    void Toggle_cb_AutoConnect(bool v);
    void Toggle_cb_Reconnect(bool v);
    void Toggle_cb_InsecureWiFi(bool v);
    void Toggle_cb_ShowNodes(bool v);
    void Toggle_cb_DisableIpv6(bool v);
    void Toggle_cb_FixDnsLeak(bool v);

    void Changed_dd_Encryption(int ix);

    void Validate_e_Dns();
    void Validate_e_SecondaryDns();
    void Validate_e_Ports();
    void Validate_e_LocalPort();

    void ToScr_Connect();
    void ToScr_Logs();
    void Clicked_Update();
};

#endif // SCR_SETTINGS_H
