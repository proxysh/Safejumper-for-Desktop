#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <memory>
#include <QSystemTrayIcon>
#include <QMainWindow>
//#include <QDialog>
#include <QCloseEvent>
#include <QTimer>
#include <QAbstractSocket>

#include "openvpnmanager.h"

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

    void StatusConnecting();
    void StatusConnected();
    void StatusDisconnected();

    void StartWifiWatcher();
    void StopWifiWatcher();
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
    void Timer_WifiWatcher();

private:
    Ui::LoginWindow *ui;
    explicit LoginWindow(QWidget *parent = 0);

    static std::auto_ptr<LoginWindow> _inst;

    bool _CancelLogin;
    bool _ConnectAfterLogin;
    void SaveCreds();
    void enableButtons(bool enabled);

    unsigned int _activatedcount;
    std::auto_ptr<QTimer> _timer_icon;
    void DisconnectIconWatcher();

    std::auto_ptr<QTimer> _timer_wifi;
    bool _wifi_processing;
};

#endif // LOGINWINDOW_H
