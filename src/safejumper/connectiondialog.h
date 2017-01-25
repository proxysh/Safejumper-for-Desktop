#ifndef SCR_CONNECT_H
#define SCR_CONNECT_H

#include "common.h"

#include <QDialog>
#include <QProcess>
#include <memory>
#include <QTcpSocket>
#include <QTimer>

namespace Ui
{
class ConnectionDialog;
}

class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:

    ~ConnectionDialog();
    static bool IsExists()
    {
        return (_inst.get() != NULL);
    }
    static ConnectionDialog * Instance();
    static void Cleanup()
    {
        if (_inst.get() != NULL) delete _inst.release();
    }

    void SetServer(int srv);	// -1 or id inside all servers
    void UpdNewIp(const QString & s);
    void UpdEnc();


    void SetVpnName(const QString & vpnname);
    void StatusConnecting();
    void StatusConnecting(const QString & word);
    void StatusConnected();
    void StatusDisconnected();

public slots:
    void SetOldIp(const QString & ip);
    void SetAccName(const QString & s);
    void SetEmail(const QString & s);
    void SetAmount(const QString & s);
    void SetUntil(const QString & date);

    void SetProtocol(int ix);   // -1 for none
    void PortDlgAction(int action);
private:
    Ui::ConnectionDialog *ui;
    static std::auto_ptr<ConnectionDialog> _inst;
    explicit ConnectionDialog(QWidget *parent = 0);
    void Init();

    void SetFlag(int srv);
    void SetNoSrv();
    void UpdProtocol();
    void SetEnabledButtons(bool enabled);
    void ModifyWndTitle(const QString & word);

    bool _moving;
    void DwnlStrs();
    QPoint _WndStart;
    QPoint _CursorStart;

    static void InitStateWords();
    typedef QHash<QString, const char *> HmWords;
    static HmWords _StateWord_Img;

private slots:
    void Clicked_Connect();
    void Clicked_Cancel();
    void Clicked_Jump();

    void Clicked_Min();
    void Clicked_Cross();

    void ToScr_Settings();
    void ToScr_Primary();
    void ToScr_Login();
    void ToScr_Map();
    void ShowPackageUrl();

    void Pressed_Head();

protected:
    virtual void closeEvent(QCloseEvent * event);
    virtual void keyPressEvent(QKeyEvent * event);
    virtual bool eventFilter(QObject *obj, QEvent *event);
};


#endif // SCR_CONNECT_H
