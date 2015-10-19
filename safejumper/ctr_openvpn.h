#ifndef CRT_OPENVPN_H
#define CRT_OPENVPN_H

#include <memory>
#include <QProcess>
#include <QTcpSocket>
#include <QFileSystemWatcher>
#include <QTemporaryFile>

#include "dlg_confirmation.h"

enum OvState
{
	ovsDisconnected = 0,
	ovsConnecting,
	ovsConnected,

	ovsTotal
};

#define G_PortQuestionDelay 60
#define G_PortIterationDelay 80

class Ctr_Openvpn
{
public:
	~Ctr_Openvpn();
	static Ctr_Openvpn * Instance();
	static void Cleanup() { if (_inst.get() != NULL) delete _inst.release();}
	static bool IsExists() { return (_inst.get() != NULL); }

	void Jump();
	void Start();
	void Start(size_t srv);
	void Cancel(const QString & msg);
	void Stop();			// normal stop of executed via this process

	bool IsOvRunning();

	void ReadStderr();
	void ReadStdout();
	void StateChanged(QProcess::ProcessState st);
	void Finished(int exitCode, QProcess::ExitStatus exitStatus);

	void Soc_Error(QAbstractSocket::SocketError er);
	void Soc_ReadyRead();

	void LogfileChanged(const QString & pfn);		// OpenVpn log file
	void CheckState();		// timer calls it

	OvState State() { return _state; }

	void KillRunningOV();		// kill process executed previously: connect to it first, if fails: kill -9

	void Timer_Reconnect();
	void StartPortLoop();
private:
	Ctr_Openvpn();
	static std::auto_ptr<Ctr_Openvpn> _inst;

	std::auto_ptr<QTemporaryFile> _paramFile;
	std::auto_ptr<QProcess> _process;
	void RemoveProcess();
	int _pid;		// for running process (run safejumper after crash)
	void AttachMgmt();	// attach to OpenVPN management socket
	std::auto_ptr<QTcpSocket> _soc;
	void RemoveSoc();
	QString _LocalAddr;	// remember at moment of OpenVPN launch - do not depend on user changes
	quint16 _LocalPort;

	std::auto_ptr<QFileSystemWatcher> _watcher;	// OpenVpn log file
	qint64 _lastpos;
	void InitWatcher();
	bool _processing;
	void ProcessOvLogLine(const QString & s);
	void ExtractNewIp(const QString & s);
	void GotConnected(const QString & s);
	void GotTunErr(const QString & s);
	bool _tunerr;
	bool _err;

	void ProcessLine(QString s);
	void ProcessRtWord(const QString & word, const QString & s);
	void ProcessPlainWord(const QString & word, const QString & s);
	void ProcessStateWord(const QString & word, const QString & s);
	QString _prev_st_word;
	void ShowErrMsgAndCleanup(QString msg);

	OvState _state;
	void SetState(OvState st);

	void ReconnectTimer();
	int _reconnect_attempt;
	bool _PortDlgShown;
	uint _dtStart;
	bool _InPortLoop;
	void StartImpl();
	void ToNextPort();
};

#endif // CRT_OPENVPN_H
