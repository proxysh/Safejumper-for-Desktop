#ifndef SCR_CONNECT_H
#define SCR_CONNECT_H

#include "common.h"

#include <QDialog>
#include <QProcess>
#include <memory>
#include <QTcpSocket>
#include <QTimer>

namespace Ui {
class Scr_Connect;
}

class Scr_Connect : public QDialog
{
	Q_OBJECT

public:

	~Scr_Connect();
	static bool IsExists() { return (_inst.get() != NULL); }
	static Scr_Connect * Instance();
	static void Cleanup() { if (_inst.get() != NULL) delete _inst.release();}

	void SetServer(int srv);	// -1 or id inside all servers
	void UpdNewIp(const QString & s);

	void SetOldIp(const QString & ip);
	void SetAccName(const QString & s);
	void SetEmail(const QString & s);
	void SetAmount(const QString & s);
	void SetUntil(const QString & date);

	void SetProtocol(int ix);   // -1 for none

	void SetVpnName(const QString & vpnname);
	void StatusConnecting();
	void StatusConnecting(const QString & word);
	void StatusConnected();
	void StatusDisconnected();
	void StartTimer();

public slots:
	void ConnectError(QProcess::ProcessError error);
	void ConnectStarted();
	void ConnectStateChanged(QProcess::ProcessState newState);
	void ConnectFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void ConnectStderr();
	void ConnectStdout();

	void LogfileChanged(const QString & pfn);
private:
	Ui::Scr_Connect *ui;
	static std::auto_ptr<Scr_Connect> _inst;
	explicit Scr_Connect(QWidget *parent = 0);
	void Init();

	void SetFlag(int srv);
	void SetNoSrv();
	void SetNoProtocol();
	void SetEnabledButtons(bool enabled);
	void ModifyWndTitle(const QString & word);

	std::auto_ptr<QTimer> _timer_state;		// check Ovpn state;

	bool _moving;
	QPoint _prevMouse;
	void DwnlStrs();

private slots:
	void Clicked_Connect();
	void Clicked_Cancel();
	void Clicked_Jump();

	void Clicked_Min();

	void ToScr_Settings();
	void ToScr_Primary();
	void ToScr_Login();
	void ToScr_Map();
	void ShowPackageUrl();

	void Pressed_Head();
	void Released_Head();

	void Timer_CheckState();
protected:
	virtual void closeEvent(QCloseEvent * event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void keyPressEvent(QKeyEvent * event);
};


#endif // SCR_CONNECT_H
