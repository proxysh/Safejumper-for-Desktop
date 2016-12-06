#ifndef SJMAINWINDOW_H
#define SJMAINWINDOW_H

#include <memory>
#include <QMainWindow>
//#include <QDialog>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QTimer>
#include <QAbstractSocket>

#include "acconnectto.h"
#include "openvpnmanager.h"

namespace Ui
{
class SjMainWindow;
}


class SjMainWindow :
    public QMainWindow
{
    Q_OBJECT

public:
    static SjMainWindow * Instance();
    static bool IsExists()
    {
        return (_inst.get() != NULL);
    }
    static void Cleanup();
    ~SjMainWindow();

    void StatusConnecting();
    void StatusConnected();
    void StatusDisconnected();

    void DoClose();

    void ConstructConnecttoMenu();
    void StartWifiWatcher();
    void StopWifiWatcher();
    void BlockOnDisconnect();

public slots:
    void AppFocusChanged(QWidget*, QWidget*);
    void LoginFinished();
    void AccTypeFinished();
    void ExpireFinished();
    void Finished_OldIp(const QString & s);
    void Finished_OldIpHttp();
    void Finished_Dns();
    void Finished_Updates();
    void Finished_ObfsName();
    void Finished_EccName();
//	void Finished_EccxorName();

    void Soc_Error(QAbstractSocket::SocketError er);
    void Soc_ReadyRead();

    // connect or login Src depending on auth status
    void ToScr_Primary();

    void ac_Connect();
    void ac_ConnectTo();
    void ac_Disconnect();
    void ac_Status();
    void ac_Jump();
    void ac_SwitchCountry();
    void ac_Settings();
    void ac_Logs();
    void ac_WebManag();
    void ac_Support();
    void ac_Bug();
    void ac_Earn();
    void ac_About();
    void ac_Logout();
    void ac_Close();

protected:
    virtual void closeEvent(QCloseEvent * event);

private slots:
    void on_rememberMeButton_toggled();
    void on_optionsButton_clicked();
    void on_loginButton_clicked();
    void on_cancelButton_clicked();

    void updateStateIcon();
    void Icon_Activated(QSystemTrayIcon::ActivationReason reason);
    void Timer_Constructed();
    void Timer_Reconnect();
    void Timer_WifiWatcher();

private:
    Ui::SjMainWindow *ui;
    explicit SjMainWindow(QWidget *parent = 0);

    static std::auto_ptr<SjMainWindow> _inst;


    std::auto_ptr<QSystemTrayIcon> _TrayIcon;
    std::auto_ptr<QMenu> _TrayMenu;

    void DisableMenuItems(bool connecting);
    std::auto_ptr<QAction> _ac_Connect;
    std::auto_ptr<QAction> _ac_ConnectTo;
    std::auto_ptr<QAction> _ac_Disconnect;
    std::auto_ptr<QAction> _ac_Status;
    std::auto_ptr<QAction> _ac_Jump;
    std::auto_ptr<QAction> _ac_SwitchCountry;
    std::auto_ptr<QAction> _ac_Settings;
    std::auto_ptr<QAction> _ac_Logs;
    std::auto_ptr<QAction> _ac_WebManag;
    std::auto_ptr<QAction> _ac_Support;
    std::auto_ptr<QAction> _ac_Bug;
    std::auto_ptr<QAction> _ac_Earn;
    std::auto_ptr<QAction> _ac_About;
    std::auto_ptr<QAction> _ac_Logout;
    std::auto_ptr<QAction> _ac_Close;

    void CreateTrayMenu();
    void CreateActions();
    void CreateTrayIcon();

    void DoLogin();
    bool _CancelLogin;
    void DoCancelLogin();
    void DoConnect();
    bool _ConnectAfterLogin;
    void SaveCreds();
    void enableButtons(bool enabled);

    void FixIcon();
    bool _fixed;
    void updateStateIcon(OvState st);
    unsigned int _activatedcount;
    std::auto_ptr<QTimer> _timer_icon;
    void DisconnectIconWatcher();

    std::auto_ptr<QTimer> _timer_wifi;
    bool _wifi_processing;

    void ClearConnecttoMenu();
    void CreateMenuItem(QMenu * m, const QString & name, size_t srv);
    std::vector<AcConnectto *> _connectto;
    std::auto_ptr<QMenu> _ct_menu;
    std::vector<QMenu *> _hub_menus;

    void DisableButtonsOnLogout();
    void EnableButtonsOnLogin();
};

#endif // SJMAINWINDOW_H
