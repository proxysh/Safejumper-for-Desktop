#include "ctr_openvpn.h"

#include <QFile>
#include <QDir>
#include <stdexcept>

#include "authmanager.h"
#include "osspecific.h"
#include "scr_connect.h"
#include "scr_map.h"
#include "wndmanager.h"
#include "setting.h"
#include "log.h"
#include "sjmainwindow.h"
#include "pathhelper.h"

Ctr_Openvpn::Ctr_Openvpn()
	: _state(ovsDisconnected)
	, _pid(0)
{}

Ctr_Openvpn::~Ctr_Openvpn()
{

}

std::auto_ptr<Ctr_Openvpn> Ctr_Openvpn::_inst;
Ctr_Openvpn * Ctr_Openvpn::Instance()
{
	if (!_inst.get())
		_inst.reset(new Ctr_Openvpn());
	return _inst.get();
}

void Ctr_Openvpn::Start(size_t srv)
{
	Scr_Map::Instance()->SetServer(srv);
	Start();
}

void Ctr_Openvpn::Start()
{
	WndManager::Instance()->ClosePortDlg();
	_PortDlgShown = false;
	_InPortLoop = false;	// TODO: -0
	_reconnect_attempt = 0;
	StartImpl();
}

void Ctr_Openvpn::StartImpl()
{
//#ifdef Q_OS_MAC
//#define NO_PARAMFILE
//#endif
	{
		Stop();
		// TODO: -1 cleanup
	}

	if (AuthManager::Instance()->IsLoggedin())
	{
		SetState(ovsConnecting);
		try
		{
			OsSpecific::Instance()->SetIPv6(!Setting::Instance()->IsDisableIPv6());
#ifdef Q_OS_WIN
			OsSpecific::Instance()->EnableTap();
#endif
		}
		catch(std::exception & ex)
		{
			log::logt(ex.what());
			if (Setting::Instance()->IsDisableIPv6())
			{
				WndManager::Instance()->ErrMsg(QString("Cannot disable IPv6 ") + ex.what());
				return;
			}
		}

		_LocalAddr = "127.0.0.1";
		_LocalPort = Setting::Instance()->LocalPort().toInt();

		AuthManager::Instance()->SetNewIp("");
		_tunerr = false;
		_err = false;
		_processing = false;
		if (NULL != _watcher.get())			// OpenVPN log file watcher
		{
			_watcher->removePath(PathHelper::Instance()->OpenvpnLogPfn());
			delete _watcher.release();
		}

		RemoveSoc();
#ifndef NO_PARAMFILE
		QFile ff(PathHelper::Instance()->OpenvpnConfigPfn());
		if (ff.open(QIODevice::WriteOnly))
		{
			ff.write("client\n");
			ff.write("dev tun\n");
			ff.write("proto "); ff.write(Setting::Instance()->Protocol().toLatin1()); ff.write("\n");		// "tcp"/"udp"
			ff.write("remote "); ff.write(Setting::Instance()->Server().toLatin1()); ff.write(" "); ff.write(Setting::Instance()->Port().toLatin1()); ff.write("\n");
			ff.write("cipher AES-256-CBC\n");
			ff.write("auth SHA512\n");
			ff.write("auth-user-pass\n");
			ff.write("remote-cert-tls server\n");
			ff.write("resolv-retry infinite\n");
			ff.write("nobind\n");
			ff.write("comp-lzo\n");
			ff.write("verb 3\n");
			ff.write("reneg-sec 0\n");
			ff.write("route-method exe\n");
			ff.write("route-delay 2\n");
			
//ff.write("dhcp-option DNS 146.185.134.104\n");
//ff.write("dhcp-option DNS 146.185.134.104\n");

			ff.flush();
			ff.close();
		}
		else
		{
			QString se = "Cannot write config file '" + PathHelper::Instance()->OpenvpnConfigPfn() + "'";
			log::logt(se);
			WndManager::Instance()->ErrMsg(se);
			return;
		}
#endif

		QStringList args;
		args
//			<< "--auth-nocache"
#ifndef NO_PARAMFILE
			<< "--config" << PathHelper::Instance()->OpenvpnConfigPfn() // /tmp/proxysh.ovpn
#endif
#ifdef NO_PARAMFILE
<< "--client"
#endif

#ifndef Q_OS_WIN
			<< "--daemon"	// does not work at windows
#endif

#ifdef NO_PARAMFILE
<< "--dev tun0"
<< "--proto" << Setting::Instance()->Protocol()
<< "--remote-random"
<< "--remote" << Setting::Instance()->Server() << Setting::Instance()->Port()

<< "--cipher" << "AES-256-CBC"
<< "--auth" << "SHA512"
<< "--remote-cert-tls" << "server"

<< "--auth-user-pass"

<< "--resolv-retry" << "infinite"
<< "--nobind"
<< "--persist-key"
<< "--persist-tun"
#endif

			<< "--ca" << PathHelper::Instance()->ProxyshCaCert()	// /tmp/proxysh.crt
#ifdef NO_PARAMFILE
<< "--verb" << "3"
<< "--comp-lzo"
<< "--route-delay" << "2"
<< "--allow-pull-fqdn"
#endif
			<< "--management" << _LocalAddr << Setting::Instance()->LocalPort()
			<< "--management-hold"
			<< "--management-query-passwords"
			<< "--log" << PathHelper::Instance()->OpenvpnLogPfn()			// /tmp/openvpn.log

//			<< "--script-security" << "3" << "system"
//			<< "--script-security" << "2" << "execve"		// https://openvpn.net/index.php/open-source/documentation/manuals/69-openvpn-21.html
			<< "--script-security" << "3"

#ifdef Q_OS_MAC			// TODO: -0 DNS on linux
			<< "--up" << PathHelper::Instance()->UpScriptPfn()				// /Applications/Safejumper.app/Contents/Resources/client.up.safejumper.sh
			<< "--down" << PathHelper::Instance()->DownScriptPfn()		// /Applications/Safejumper.app/Contents/Resources/client.down.safejumper.sh
#endif
			<< "--up-restart"

		;

		if (!Setting::Instance()->Dns1().isEmpty())
			args << "--dhcp-option" << "DNS" << Setting::Instance()->Dns1();
		if (!Setting::Instance()->Dns2().isEmpty())
			args << "--dhcp-option" << "DNS" << Setting::Instance()->Dns2();

		Scr_Connect * sc = Scr_Connect::Instance();
		QString prog = PathHelper::Instance()->OpenvpnPathfilename();
		log::logt("Prog is: " + prog);
		QString params = args.join(' ');
		log::logt("Args are:" + params);

		bool ok = false;
		try
		{
#ifndef Q_OS_WIN
			_paramFile.reset(new QTemporaryFile());
			if (!_paramFile->open())
				throw std::runtime_error("Cannot create tmp file.");

			OsSpecific::Instance()->SetRights();		// lean inside, throw on error

			_paramFile->write(params.toLatin1());
			_paramFile->flush();

			QStringList arg3;
			arg3 << _paramFile->fileName();
			_paramFile->close();

//#ifdef Q_OS_MAC
//			log::logt("######  touch launcher ####");
//			OsSpecific::Instance()->RunFastCmd("touch -a " + PathHelper::Instance()->LauncherPfn());
//#endif
			log::logt("######  before exec ####");

//#ifdef Q_OS_MAC
//			OsSpecific::Instance()->ExecAsRoot(prog, args);		// force password dialog; without launcher
//#else
			int r3 = QProcess::execute(PathHelper::Instance()->LauncherPfn(), arg3);	// 30ms block internally
			log::logt("QProcess::execute() returns " + QString::number(r3));
			log::logt("###############");
			if (r3 != 0)
			{
				std::string ts;
				switch (r3)
				{
					case -2:		// cannot be started
					case -1:		// the process crashes
					{
						ts = ("OpenVPN couldn't be reached (" + QString::number(r3) + "). Please reboot and/or re-install Safejumper.").toStdString();
						break;
					}
					default:
						ts = ("Cannot run OpenVPN. Error code is: " + QString::number(r3)).toStdString();
						break;
				}
				throw std::runtime_error(ts);
			}
//#endif

			AttachMgmt();	// TODO: -1 wait for slow starting cases
#else
			_process.reset(new QProcess());
			sc->connect(_process.get(), SIGNAL(error(QProcess::ProcessError)), sc, SLOT(ConnectError(QProcess::ProcessError)));
			sc->connect(_process.get(), SIGNAL(started()), sc, SLOT(ConnectStarted()));
			sc->connect(_process.get(), SIGNAL(stateChanged(QProcess::ProcessState)), sc, SLOT(ConnectStateChanged(QProcess::ProcessState)));
			sc->connect(_process.get(), SIGNAL(finished(int, QProcess::ExitStatus)), sc, SLOT(ConnectFinished(int, QProcess::ExitStatus)));
			sc->connect(_process.get(), SIGNAL(readyReadStandardError()), sc, SLOT(ConnectStderr()));
			sc->connect(_process.get(), SIGNAL(readyReadStandardOutput()), sc, SLOT(ConnectStdout()));
			_process->start(prog, args);
			_process->waitForStarted(2000);
			log::logt("Process ID is: " + QString::number(_process->processId()));
#endif
			ok = true;
		}
		catch(std::exception & ex)
		{
			log::logt(ex.what());
			WndManager::Instance()->ErrMsg(ex.what());
		}

		if (_paramFile.get())
			delete _paramFile.release();

		if (ok)
		{
			_dtStart = QDateTime::currentDateTimeUtc().toTime_t();
			Scr_Connect::Instance()->StartTimer();
		}
		else
			SetState(ovsDisconnected);
	}
#undef NO_PARAMFILE
}

void Ctr_Openvpn::CheckState()
{
	if (IsOvRunning())
	{
		if (NULL != _soc.get())
		{
			if (_soc->isOpen() && _soc->isValid())
			{
				if (_soc->state() == QAbstractSocket::ConnectedState)
				{
					_soc->write("state\n");
					_soc->flush();
				}
			}
		}
		else
		{	// first time
			InitWatcher();
			AttachMgmt();
		}
	}
	else
	{
		if (State() == ovsConnected)
		{
			// handle crush
			if (Setting::Instance()->IsReconnect())
				Start();
			else
				SetState(ovsDisconnected);
		}
	}

	if (State() == ovsConnecting)
	{
		uint d = QDateTime::currentDateTimeUtc().toTime_t() - _dtStart;
		if (!_PortDlgShown && !_InPortLoop)
		{
			if (d > G_PortQuestionDelay)
			{
				_PortDlgShown = true;
				WndManager::Instance()->ShowPortDlg();
			}
		}

		if (_InPortLoop)
		{
			if (d > G_PortIterationDelay)
				ToNextPort();
		}
	}
}

void Ctr_Openvpn::LogfileChanged(const QString & pfn)
{
	if (_processing || _err)
		return;
	_processing = true;
	if (pfn == PathHelper::Instance()->OpenvpnLogPfn())
	{
		QFile f(pfn);
		QByteArray ba;
		if (f.open(QIODevice::ReadOnly))		// TODO: -2 ensure non-blocking
		{
			if (_lastpos > f.size())		// file was truncated
				_lastpos = 0;
			if ((_lastpos + 1) < f.size())
			{
				f.seek(_lastpos);
				ba = f.read(f.size() - _lastpos);
				_lastpos = f.size();
			}
			f.close();
		}
		if (!ba.isEmpty())
		{
			QString s1(ba);
			QStringList sl = s1.split('\n', QString::SkipEmptyParts);
			for (int k = 0; k < sl.size(); ++k)
				ProcessOvLogLine(sl[k]);
		}
	}
	_processing = false;
}

void Ctr_Openvpn::ProcessOvLogLine(const QString & s)
{
	// TODO: -2 state machine
	if (s.contains("MANAGEMENT: CMD 'state'", Qt::CaseInsensitive))
	{
		;	// skip our commands
	} else {
		log::logt("OpenVPNlogfile: " + s);
	if (s.contains("TCPv4_CLIENT link remote:", Qt::CaseInsensitive)) {
		ExtractNewIp(s);
	} else { if (s.contains("Initialization Sequence Completed:", Qt::CaseInsensitive)) {
		GotConnected(s);
	} else { if (s.contains("Opening utun (connect(AF_SYS_CONTROL)): Operation not permitted", Qt::CaseInsensitive)) {
		GotTunErr(s);
	}}}}
}

void Ctr_Openvpn::SetState(OvState st)
{
	if (st != _state)
	{
		_state = st;
log::logt("Set state " + QString::number(st));
		switch (st)
		{
			case ovsConnected:
			{
				WndManager::Instance()->HandleConnected();
				break;
			}
			case ovsConnecting:
			{
				WndManager::Instance()->HandleConnecting();
				break;
			}
			case ovsDisconnected:
			{
				if (Setting::Instance()->IsBlockOnDisconnect())
					SjMainWindow::Instance()->BlockOnDisconnect(true);
				WndManager::Instance()->HandleDisconnected();
				break;
			}
			default:
				break;
		}
	}
}

void Ctr_Openvpn::GotConnected(const QString & s)
{
	SetState(ovsConnected);
	// extract IP
	//1432176303,CONNECTED,SUCCESS,10.14.0.6,91.219.237.159
	int p = -1;
	for (int k = 0; k < 4; ++k)
		p = s.indexOf(',', p + 1);
	if (p > -1)
	{
		QString ip = s.mid(p + 1);
		AuthManager::Instance()->SetNewIp(ip);
	}

	AuthManager::Instance()->ForwardPorts();
}

void Ctr_Openvpn::GotTunErr(const QString & s)
{
	if (!_tunerr && !_err)
	{
		_tunerr = true;
		_err = true;
		this->Cancel(s);
	}
}

void Ctr_Openvpn::ExtractNewIp(const QString & s)
{
	// Tue May 1 03:50:58 2015 TCPv4_CLIENT link remote: [AF_INET]50.31.252.10:443
	int p0 = s.indexOf("TCPv4_CLIENT link remote");
	p0 = s.indexOf(':', p0 + 1);
	if (p0 > -1)
	{
		int p1 = s.indexOf(':', p0 + 1);
		if (p1 > -1)
		{

			int points[3];
			points[0] = s.indexOf('.', p0);
			int p2 = points[0] - 1;
			for (; p2 > p0; --p2)
			{
				if (!s[p2].isDigit())
					break;
			}
			QString ip = s.mid(p2 + 1, p1 - p2 - 1);
			AuthManager::Instance()->SetNewIp(ip);
			Scr_Connect::Instance()->UpdNewIp(ip);
		}
	}
}

void Ctr_Openvpn::Cancel(const QString & msg)
{
	Stop();
	WndManager::Instance()->ErrMsg(msg);
}

/*
void Ctr_Openvpn::Cancel()
{
	if (IsOvRunning())
		Stop();
	SetState(ovsDisconnected);
}
*/
void Ctr_Openvpn::Stop()
{
	if (IsOvRunning())
	{
		if (NULL != _soc.get())
		{
			if (_soc->isOpen() && _soc->isValid())
			{
				if (_soc->state() != QAbstractSocket::ConnectedState)
				{
					log::logt("Cannot send signal SIGTERM due to disconnected socket");
				}
				else
				{
					log::logt("signal SIGTERM");
					//_soc->write("echo all\n");
					//_soc->flush();

					_soc->write("signal SIGTERM\nexit\n");
					_soc->flush();
					_soc->close();
				}
			}
			QObject * to = _soc.release();
			to->deleteLater();

			for (int cn = 0; cn < 8 && IsOvRunning(); ++cn)
				QThread::msleep(100);		// just sleep now; without this delay it fails to jump
		}

		QThread::msleep(200);
		RemoveProcess();

		if (IsOvRunning())
			log::logt("Stop(): cannot soft stop OpenVPN process");
	}
	SetState(ovsDisconnected);
}

void Ctr_Openvpn::RemoveProcess()
{
	if (_process.get() != NULL)
	{
		QProcess * t = _process.release();
		t->deleteLater();
	}
}

void Ctr_Openvpn::ReadStderr()
{
	log::logt("ReadStderr(): " + _process->readAllStandardError());
}

void Ctr_Openvpn::ReadStdout()
{
	log::logt("ReadStdout(): " + _process->readAllStandardOutput());
}

void Ctr_Openvpn::StateChanged(QProcess::ProcessState st)
{
	// TODO: handle open vpn  process startup
	InitWatcher();
}

void Ctr_Openvpn::Finished(int exitCode, QProcess::ExitStatus exitStatus)
{
	// OpenVpn crushed or just spawn a child and exit during startup
	if (exitCode != 0)	// TODO: -1 handle open vpn process startup
	{
		// TODO: -1 handle used socket
		// MANAGEMENT: Socket bind failed on local address [AF_INET]127.0.0.1:6842: Address already in use

// TODO: -0 delete or not? it spawns child and exits
//		RemoveProcess();
		if (!IsOvRunning())
			SetState(ovsDisconnected);
	}
	else
	{
		InitWatcher();
		AttachMgmt();
	}
}

void Ctr_Openvpn::InitWatcher()
{
	if (_watcher.get() == NULL)
	{
		QFile f(PathHelper::Instance()->OpenvpnLogPfn());
		if (f.exists())
		{
			_lastpos = 0;		// OpenVpn will truncate

			Scr_Connect * sc = Scr_Connect::Instance();
			_watcher.reset(new QFileSystemWatcher());
			_watcher->addPath(PathHelper::Instance()->OpenvpnLogPfn());

			log::logt("Monitoring " + PathHelper::Instance()->OpenvpnLogPfn());
			sc->connect(_watcher.get(), SIGNAL(fileChanged(const QString &)), sc, SLOT(LogfileChanged(const QString &)));
		}
	}
}

void Ctr_Openvpn::RemoveSoc()
{
	if (NULL != _soc.get())
	{
		SjMainWindow * sc = SjMainWindow::Instance();
		sc->disconnect(_soc.get(), SIGNAL(error(QAbstractSocket::SocketError)), sc, SLOT(Soc_Error(QAbstractSocket::SocketError)));
		sc->disconnect(_soc.get(), SIGNAL(readyRead()), sc, SLOT(Soc_ReadyRead()));
		_soc->abort();
		_soc.release()->deleteLater();
	}
}

void Ctr_Openvpn::AttachMgmt()
{
	RemoveSoc();
	_soc.reset(new QTcpSocket());
	SjMainWindow * sc = SjMainWindow::Instance();
	sc->connect(_soc.get(), SIGNAL(error(QAbstractSocket::SocketError)), sc, SLOT(Soc_Error(QAbstractSocket::SocketError)));
	sc->connect(_soc.get(), SIGNAL(readyRead()), sc, SLOT(Soc_ReadyRead()));
	_soc->connectToHost(_LocalAddr, _LocalPort);
}

void Ctr_Openvpn::Soc_Error(QAbstractSocket::SocketError er)
{
	log::logt("Error connecting to OpenVPN management socket");
	if (NULL != _soc.get())
		_soc.release()->deleteLater();
}

void Ctr_Openvpn::Soc_ReadyRead()
{
//	log::logt("Soc_ReadyRead()");
	if (!_soc->canReadLine())
		return;
	QString s = _soc->readAll();
	_soc->flush();

	QStringList sl = s.split('\n', QString::SkipEmptyParts);
	for (int k = 0; k < sl.size(); ++k)
		ProcessLine(sl[k].trimmed());
}

void Ctr_Openvpn::ProcessLine(QString s)
{
	// TODO: -1 hash_map
	// TODO: -2 state machine

	if (s.startsWith('>'))
	{	// >PASSWORD:Need 'Auth' username/password
		int p = s.indexOf(':');
		if (p > -1)
		{
			QString word = s.mid(1, p - 1);
			ProcessRtWord(word, s);
		}
		else
		{
			// without :
			// just ignore
		}
	}
	else
	{	// not starting with >
		QString word;
		if (s[0].isDigit())
		{
			// 1432065857,GET_CONFIG,,,
			int p[4];
			p[0] = s.indexOf(',', 1);
			p[1] = s.indexOf(',', p[0] + 1);
			if (p[1] > -1)
			{
				word = s.mid(p[0] + 1, p[1] - p[0] - 1);

				// check error
				// 1436967388,CONNECTED,ERROR,10.9.0.6,158.255.211.19
				int p3 = s.indexOf(',', p[1] + 1);
				bool err = false;
				if (p3 > -1)
				{
					if (p3 > p[1] + 1)
					{
						QString third = s.mid(p[1] + 1, p3 - p[1] - 1);
						if (third.contains("ERROR", Qt::CaseInsensitive))
							err = true;
					}
				}
				if (!err)
					ProcessStateWord(word, s);
			}
			else
			{
				// TODO: -2
				word = s;
				ProcessPlainWord(word, s);
			}
		}
		else
		{
			int p = s.indexOf(':');
			if (p > -1)
			{	// SUCCESS: 'Auth' password entered, but not yet verified
				word = s.mid(0, p);
			}
			else
			{
				word = s;
			}
			ProcessPlainWord(word, s);
		}
	}
}

void Ctr_Openvpn::ProcessStateWord(const QString & word, const QString & s)
{
	// TODO: -1 hash_map
	// TODO: -2 state machine

	bool isnew = false;
	if (word != _prev_st_word)
	{
		log::logt(s);
		log::logt("processing state word '" + word + "'");
		isnew = true;
	}
	if (word.compare("CONNECTED", Qt::CaseInsensitive) == 0) {
		if (isnew)
			GotConnected(s);
		else
		{
//			log::logt("isnew = false; word = " + word + " _prev_st_word = " + _prev_st_word);
		}
	} else { if (word.compare("CONNECTING", Qt::CaseInsensitive) == 0) {
		SetState(ovsConnecting);
		WndManager::Instance()->HandleState(word);
	} else { if (word.compare("WAIT", Qt::CaseInsensitive) == 0) {
		WndManager::Instance()->HandleState(word);
	} else { if (word.compare("AUTH", Qt::CaseInsensitive) == 0) {
		WndManager::Instance()->HandleState(word);
	} else { if (word.compare("EXITING", Qt::CaseInsensitive) == 0) {
		if (Setting::Instance()->IsReconnect())
		{	// initiate autoreconnect
			ReconnectTimer();
			WndManager::Instance()->HandleConnecting();
		}
		else
		{
			WndManager::Instance()->HandleDisconnected();
		}
	} else { if (word.compare("RECONNECTING", Qt::CaseInsensitive) == 0) {
		SetState(ovsConnecting);
		WndManager::Instance()->HandleState(word);
	} else { if (word.compare("AUTH", Qt::CaseInsensitive) == 0) {
		SetState(ovsConnecting);
		WndManager::Instance()->HandleState(word);
	} else { if (word.compare("GET_CONFIG", Qt::CaseInsensitive) == 0) {
		WndManager::Instance()->HandleState(word);
	} else { if (word.compare("ASSIGN_IP", Qt::CaseInsensitive) == 0) {
		SetState(ovsConnecting);
		WndManager::Instance()->HandleState(word);
	} else { if (word.compare("TCP_CONNECT'", Qt::CaseInsensitive) == 0) {
		WndManager::Instance()->HandleState(word);
	} else {
		WndManager::Instance()->HandleState(word);
	}}}}}}}}}}
	_prev_st_word = word;
}

void Ctr_Openvpn::ProcessPlainWord(const QString & word, const QString & s)
{
	// TODO: -1 hash_map
	// TODO: -2 state machine
	if (word.compare("END", Qt::CaseInsensitive) == 0) {
		;
	} else {
		log::logt(s);
		log::logt("processing plain word '" + word + "'");
		if (word.compare("SUCCESS", Qt::CaseInsensitive) == 0) {
		;
	} else { if (word.compare("ERROR", Qt::CaseInsensitive) == 0) {
		WndManager::Instance()->ErrMsg(s);
	}}}
}

void Ctr_Openvpn::ProcessRtWord(const QString & word, const QString & s)
{
	// TODO: -1 hash_map
	// TODO: -2 state machine

	log::logt(s);
	log::logt("processing RT word '" + word + "'");

	if (word.compare("INFO", Qt::CaseInsensitive) == 0) {
		// INFO:
		// just ignore
	} else { if (word.compare("HOLD", Qt::CaseInsensitive) == 0)
	{
		if (s.indexOf("hold release") > -1)
		{
			log::logt("hold off");
			_soc->write("hold off\n");
			log::logt("hold release");
			_soc->write("hold release\n");
			_soc->flush();
		}
	} else { if (word.compare("PASSWORD", Qt::CaseInsensitive) == 0) {
		if (s.indexOf("Need 'Auth' username/password") > -1)
		{
			log::logt("sending vpn login+password");
			QString u1 = "username \"Auth\" \"" + EscapePsw(AuthManager::Instance()->VpnName()) + "\"\n";
			QString p1 = "password \"Auth\" \"" + EscapePsw(AuthManager::Instance()->VpnPsw()) + "\"\n";
			_soc->write(u1.toLatin1());
			_soc->write(p1.toLatin1());
			_soc->flush();
		}
		else
		{
			int p = s.indexOf(':');
			if (s.indexOf("Verification Failed", p, Qt::CaseInsensitive) > -1)
			{
				_err = true;
				// OpenVpn exiting
				ShowErrMsgAndCleanup(s.mid(p + 1));
			}
		}
	} else { if (word.compare("SUCCES", Qt::CaseInsensitive) == 0) {
		;
	} else { if (word.compare("FATAL", Qt::CaseInsensitive) == 0) {
		_err = true;
		int p = s.indexOf(':');
		QString msg = s.mid(p + 1);
		this->Cancel(msg);
	}}}}}
}

void Ctr_Openvpn::ShowErrMsgAndCleanup(QString msg)
{
	RemoveSoc();
	RemoveProcess();

	SetState(ovsDisconnected);
	WndManager::Instance()->ErrMsg(msg);
}

bool Ctr_Openvpn::IsOvRunning()
{
	bool is = false;

#define AA() do { log::logt("iov: " + QString::number(__LINE__)); } while(0)
#undef AA
#define AA() {}
AA();
	if (NULL != _process.get())
	{
AA();		if (QProcess::Running == _process->state() || QProcess::Starting == _process->state())
		{
AA();			is = true;
		}
		else
		{
AA();
//is = true;		// HACK: -0 locate OpenVpn child process
		}
AA();	}
AA();
	if (!is)
	if (NULL != _soc.get())
	{
AA();		if (_soc->isOpen())
{AA();			is = true;
}	}
AA();
//	if (!is)		// lookup child
	{
#ifdef WIN32
		   ;        // TODO: -0
#else

/*	command composition '|' does not work here
	QString sout = OsSpecific::Instance()->RunFastCmd(OsSpecific::Instance()->IsRunningCmd());
	QString s3 = sout.trimmed();
	int p = s3.indexOf(' ');
	QString s4 = s3.mid(0, p);
	bool converted;
	_pid = s4.toInt(&converted);
	if (converted)
	{
		if (_pid > 0)
			is = true;
	}
	return is;
*/

AA();		QTemporaryFile file(QDir::tempPath() + "/safejumper-tmp-XXXXXX.sh");
		QTemporaryFile outf(QDir::tempPath() + "/safejumper-tmp-XXXXXX.out");
		if (file.open())
		if (outf.open())
		{
AA();			QString script = QString(OsSpecific::Instance()->IsRunningCmd()) + " > " + outf.fileName();
			file.write(script.toLatin1());
			file.flush();


AA();			int re = QProcess::execute("/bin/bash", QStringList() << file.fileName());
AA();			switch (re)
			{
				case -2: log::logt("IsOvRunning(): -2 the process cannot be started");
AA();					break;
				case -1: log::logt("IsOvRunning(): -1 the process crashes");
					break;
				case 0:
				{
AA();						QByteArray ba = outf.readAll();
						QString s2(ba);
						QString s3 = s2.trimmed();
						int p = s3.indexOf(' ');
						QString s4 = s3.mid(0, p);
						bool converted;
						_pid = s4.toInt(&converted);
AA();						if (converted)
						{
AA();							if (_pid > 0)
							{
AA();								is = true;
//								AttachMgmt();
AA();							}
						}
AA();						break;
				}
				case 1: is = false;		// no lines
					break;
				case 2: is = false;		// grep failure
					break;
				default:
AA();					log::logt("IsOvRunning(): ps-grep return code = " + QString::number(re));
					break;
			}
		}
#endif  // else WIN32
	}
//log::logt(QString("IsOvRunning() returns ") + QString(is ? "true": "false") );
AA();	return is;
}

void Ctr_Openvpn::KillRunningOV()
{
log::logt(QString("KillRunningOV() enter"));
	if (IsOvRunning())
	{
		AttachMgmt();
		if (_soc.get())
		for (int count = 0; count < 8 && !_soc->isOpen() && !_soc->isValid(); ++count)
			QThread::msleep(100);		// HACK: -0 just timeout instead of waiting for connection;

		Stop();		// soft stop

		if (IsOvRunning())
		{
			// TODO: -0 kill -9 prog

			if (_pid > 0)
			{
				QStringList a;
				a << "-9" << QString::number(_pid);
				try
				{
					OsSpecific::Instance()->ExecAsRoot("/bin/kill", a);
				}
				catch(std::exception & ex)
				{
					log::logt(ex.what());
					WndManager::Instance()->ErrMsg(ex.what());
				}
			}
		}
	}
log::logt(QString("KillRunningOV() exit"));
}

void Ctr_Openvpn::StartPortLoop()
{
	if (State() != ovsConnected)		// if not connected already during this call
	{
		_InPortLoop = true;
		uint dt = QDateTime::currentDateTimeUtc().toTime_t();
		if ((dt - _dtStart) > G_PortQuestionDelay)
		{
			ToNextPort();
		}
	}
}

void Ctr_Openvpn::ToNextPort()
{
	_dtStart = QDateTime::currentDateTimeUtc().toTime_t();		// force start interval - prevent double port change
	Setting::Instance()->SwitchToNextPort();
	StartImpl();
}

void Ctr_Openvpn::ReconnectTimer()
{
	QTimer::singleShot(1000, SjMainWindow::Instance(), SLOT(Timer_Reconnect()));
}

void Ctr_Openvpn::Timer_Reconnect()
{
	++_reconnect_attempt;
	if (IsOvRunning())
	{
		if (_reconnect_attempt < 20)
			QTimer::singleShot(200, SjMainWindow::Instance(), SLOT(Timer_Reconnect()));
		else
		{
			Stop();
			StartImpl();	// force stop then start
		}
	}
	else
	{
		StartImpl();
	}
}

