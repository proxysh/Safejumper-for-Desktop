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

#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <memory>
#include <QSystemTrayIcon>
#include <QMainWindow>
//#include <QDialog>
#include <QCloseEvent>
#include <QTimer>
#include <QAbstractSocket>

namespace Ui
{
class LoginWindow;
}

class LoginWindow :
    public QMainWindow
{
    Q_OBJECT

public:
    static LoginWindow * Instance();
    static bool IsExists()
    {
        return (_inst.get() != NULL);
    }
    static void Cleanup();
    ~LoginWindow();

    void startWifiWatcher();
    void stopWifiWatcher();
    void BlockOnDisconnect();

public slots:
    void loggedIn();
    void loginError(QString message);
    void quitApplication();
//	void Finished_EccxorName();

    // connect or login Src depending on auth status
    void ToScr_Primary();

protected:
    virtual void closeEvent(QCloseEvent * event);

private slots:
    void on_rememberMeButton_toggled();
    void on_optionsButton_clicked();
    void on_loginButton_clicked();
    void on_cancelButton_clicked();

    void Timer_Constructed();
    void checkWifi();

    void detectInsecureWifiChanged();
private:
    Ui::LoginWindow *ui;
    explicit LoginWindow(QWidget *parent = 0);

    static std::auto_ptr<LoginWindow> _inst;

    bool _CancelLogin;
    bool _ConnectAfterLogin;
    void saveCredentials();
    void enableButtons(bool enabled);

    unsigned int _activatedcount;
    std::auto_ptr<QTimer> _timer_icon;
    void DisconnectIconWatcher();

    std::auto_ptr<QTimer> _timer_wifi;
    bool _wifi_processing;
    bool mQuitConfirmed;
};

#endif // LOGINWINDOW_H
