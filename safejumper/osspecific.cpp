#include "osspecific.h"

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QProcess>
#include <QThread>
#include <QSysInfo>

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <windows.h>
#include <Iphlpapi.h>
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#endif

#ifdef Q_OS_MAC
#include <Security/Authorization.h>
#include <Security/AuthorizationTags.h>
#endif

#include <QtCore/qshareddata.h>
#include <QtCore/qscopedpointer.h>
#include <QtNetwork/qhostaddress.h>

#include <QNetworkInterface>

#include <exception>
#include <stdexcept>

//#include <sys/unistd.h>		// chown
#include <sys/stat.h>		// chmod
#include <errno.h>

#include "common.h"
#include "log.h"
#include "sjmainwindow.h"
#include "wndmanager.h"
#include "pathhelper.h"



std::auto_ptr<OsSpecific> OsSpecific::_inst;
OsSpecific * OsSpecific::Instance()
{
	if (!_inst.get())
		_inst.reset(new OsSpecific());
	return _inst.get();
}

OsSpecific::OsSpecific()
	: _netdown(false)
{}

bool _auth(false);
#ifdef Q_OS_MAC
AuthorizationRef _AuthorizationRef;
// throw std::exception on error
AuthorizationRef & GetAuth()
{
	if (!_auth)
	{
		OSStatus res;
		QString msg = "Safejumper requires to make some modifications to your network settings.\n\n";
		QByteArray ba = msg.toUtf8();
		AuthorizationItem environmentItems[] = {
		{kAuthorizationEnvironmentPrompt, (size_t)ba.size(), (void*)ba.data(), 0},
	//	        {kAuthorizationEnvironmentIcon, iconPathLength, (void*)iconPathC, 0}
		};
		AuthorizationEnvironment myEnvironment = {1, environmentItems};
		AuthorizationFlags myFlags = kAuthorizationFlagDefaults;
		res = AuthorizationCreate(NULL, &myEnvironment, myFlags, &_AuthorizationRef);
		if (res != errAuthorizationSuccess)
			throw std::runtime_error(("AuthorizationCreate() fails with result: " + QString::number(res)).toStdString());

		AuthorizationItem myItems = {kAuthorizationRightExecute, 0, NULL, 0};
		AuthorizationRights myRights = {1, &myItems};
		myFlags = kAuthorizationFlagDefaults |
			kAuthorizationFlagInteractionAllowed |
	//		kAuthorizationFlagPreAuthorize |
			kAuthorizationFlagExtendRights;

		res = AuthorizationCopyRights(_AuthorizationRef, &myRights, &myEnvironment, myFlags, NULL);
		if (res != errAuthorizationSuccess)
			throw std::runtime_error(("AuthorizationCopyRights() fails with result: " + QString::number(res)).toStdString());
		_auth = true;
	}
	return _AuthorizationRef;
}

void OsSpecific::SetOwnerRoot(const QString & pfn)
{
	// check if bit set already
	if (!IsOwnerRoot(pfn))
	{
#ifdef Q_OS_WIN
#error SetOwnerRoot() Not implemented
		throw std::runtime_error("SetOwnerRoot() Not implemented");
#endif
		AuthorizationRef & ra = GetAuth();
//		QStringList args;
//		args << "u+s" << pfn;
//		ExecAsRoot("chmod", args);

		//int	chmod(const char *, mode_t) __DARWIN_ALIAS(chmod);
		int r1 = chmod(pfn.toLatin1(), 04555);
		if (r1 != 0)
		{
			QString s("cannot set 04555! err code = ");
			s += QString::number(errno);
			log::logt(s);	//throw std::runtime_error("cannot set 04555!");
		}
/*
		int r2 = chown(pfn.toLatin1(), 0, 500);
		if (r2 != 0)
		{
			QString s("cannot ch own 0 0! err code = ");
			s += QString::number(errno);
			log::logt(s);	//throw std::runtime_error("cannot ch own 0 0!");
		}
*/
/*	{
			log::logt(pfn);

		QStringList args2;
		args2
				//<< "ls"
			  << "-l" << "/tmp";
		ExecAsRoot("ls", args2);
	}
*/
		try
		{
			QStringList args;
			args << "root" << pfn;
			ExecAsRoot("/usr/sbin/chown", args);


			// additional files
		}
		catch(std::exception & ex)
		{
			// do not propagate - allow different method of OpenVPN execution regardless this property change
			log::logt(QString("Cannot set owner of ") + pfn + "err = " + ex.what());
		}
	}
}
#endif

OsSpecific::~OsSpecific()
{
	if (_auth)
	{
#ifdef Q_OS_MAC
		OSStatus res = AuthorizationFree(_AuthorizationRef, kAuthorizationFlagDefaults);
#endif
		_auth = false;
	}
}

bool OsSpecific::IsOwnerRoot(const QString & pfn)
{
	QFileInfo fi(pfn);
	return fi.exists() && fi.ownerId() == 0;
}

void OsSpecific::ReleaseRights()
{
	if (_auth)
	{
#ifdef Q_OS_MAC
		AuthorizationFree(_AuthorizationRef, kAuthorizationFlagDefaults);
#endif
		_auth = false;
	}
}

static void ExecAsRootMac(const QString & cmd, char * const * argv)
{
	//execute
#ifdef Q_OS_MAC
	AuthorizationRef & ra = GetAuth();
	OSStatus res = AuthorizationExecuteWithPrivileges(ra,
		 cmd.toStdString().c_str()	//myToolPath
		 , kAuthorizationFlagDefaults,
		 argv									//myArguments
		 , NULL);							// pipes

	// handle misterious errors with auth service
	for (int k = 1; k < 6 && errAuthorizationToolEnvironmentError == res; ++k)		// 1500ms  total
	{
		int ms = k * 100;
		log::logt("Auth - got errAuthorizationToolEnvironmentError, sleep " + QString::number(ms) + "ms");
		QThread::msleep(ms);
		res = AuthorizationExecuteWithPrivileges(ra,
				 cmd.toStdString().c_str()	//myToolPath
				 , kAuthorizationFlagDefaults,
				 argv									//myArguments
				 , NULL);							// pipes
	}

	if (res != errAuthorizationSuccess)
		throw std::runtime_error(("AuthorizationExecuteWithPrivileges() fails with result: "
			+ QString::number(res) + " cmd = " + cmd).toStdString());
#endif	// Q_OS_MAC
}

static void ExecAsRootUbuntu(const QString & cmd, const QStringList & args)
{
	QStringList args1;
	args1 << cmd << args;
	int re = QProcess::execute("pkexec", args1);
	if (re != 0)
	{
		QString s1 = "Exec as root fails. result code = " + QString::number(re);
		if (re == 126)
			s1 = " User declined auth";
		throw std::runtime_error(s1.toStdString());
	}
}

void OsSpecific::ExecAsRoot(const QString & cmd, const QStringList & args)
{
	{
		QString m = "Executing as root. Cmd = " + cmd + " args = ";
		m += args.join(' ');
		log::logt(m);
	}

#ifdef Q_OS_MAC
	std::vector<const char *> argv1;
	std::vector<std::string> argv_store;
	argv1.reserve(args.size() + 1);
	argv_store.reserve(args.size());
	for (int k = 0; k < args.size(); ++k)
	{
		argv_store.push_back(args.at(k).toStdString());
		std::string & ts = argv_store.at(k);
		argv1.push_back(ts.c_str());
	}
	argv1.push_back(NULL);

	ExecAsRootMac(cmd, (char * const *)&argv1[0]);
#else		// Q_OS_MAC
#ifdef Q_OS_LINUX
	ExecAsRootUbuntu(cmd, args);
#else
	throw std::runtime_error("non Mac, non Linux exec as root");
#endif	//Q_OS_LINUX
#endif	// Q_OS_MAC
	// bring our window back to front after OS password dialog
	WndManager::Instance()->ToFront();

	log::logt("ExecAsRoot() success");
}

void OsSpecific::SetRights()
{
	// Qt uses permissions in HEX while OS's in 8-based !


	// will ask for elevated rights inside
#ifdef Q_OS_MAC

	SetChmod("0744", PathHelper::Instance()->UpScriptPfn());
	SetChown(PathHelper::Instance()->UpScriptPfn());
	SetChmod("0744", PathHelper::Instance()->DownScriptPfn());
	SetChown(PathHelper::Instance()->DownScriptPfn());

	SetChmod("0755", PathHelper::Instance()->OpenvpnPathfilename());
	SetChown(PathHelper::Instance()->OpenvpnPathfilename());

	SetChmod("04555", PathHelper::Instance()->NetDownPfn());
	SetChown(PathHelper::Instance()->NetDownPfn());

	SetChmod("04555", PathHelper::Instance()->LauncherPfn());
	SetChown(PathHelper::Instance()->LauncherPfn());

#endif		// Q_OS_MAC

#ifdef Q_OS_LINUX
	SetChown(PathHelper::Instance()->LauncherPfn());
	SetChmod("04755", PathHelper::Instance()->LauncherPfn());				// odrer is important
	
	SetChown(PathHelper::Instance()->NetDownPfn());
	SetChmod("04755", PathHelper::Instance()->NetDownPfn());
#endif

	//ReleaseRights();

	QThread::msleep(200);		// HACK: -1 wait till file system changes become effective
}

void OsSpecific::SetChmod(const char * sflags, const QString & pfn)
{
	struct stat st;		// QFileInfo fails often
	if (0 != stat(pfn.toStdString().c_str(), &st))
		throw std::runtime_error(("Cannot read flags on file " + pfn).toStdString());

	bool ok;
	QString qs(sflags);
	//unsigned flags16 = qs.toInt(&ok, 16);
	//if (!ok) throw std::runtime_error((QString("Internal error: Cannot atoi ") + sflags).toStdString());
	unsigned flags8 = qs.toInt(&ok, 8);
	if (!ok) throw std::runtime_error((QString("Internal error: Cannot atoi ") + sflags).toStdString());

	if ((st.st_mode & flags8) != flags8)
	{
		QStringList args1;
		args1 << sflags << pfn;
		ExecAsRoot("/bin/chmod", args1);				// mac, ubuntu

//		QStringList a;		// HACK: -1 wait till file system changes become effective
//		a << pfn;
//		//QProcess::execute("/usr/bin/touch", a);
//		ExecAsRoot("/usr/bin/touch", a);

		// HACK: wait till file system changes become effective
		int r2 = stat(pfn.toStdString().c_str(), &st);
		for (int k = 1; k < 6 && 0 == r2 && ((st.st_mode & flags8) != flags8) ; ++k)
		{
			QThread::msleep(k * 100);
			r2 = stat(pfn.toStdString().c_str(), &st);
		}
	}
	return;

	/*
	QFileInfo fi(pfn);
	fi.refresh();
	if (!fi.exists())
		throw std::runtime_error(("Cannot chmod. File does not exist " + pfn).toStdString());
QFile::Permissions old = fi.permissions();
	if (!fi.permission((QFile::Permissions)flags))
	{
		AuthorizationRef & a = GetAuth();
		if (!QFile::setPermissions(pfn, (QFile::Permissions)flags))
			throw std::runtime_error(("Cannot chmod " + QString::number(flags, 16) + " on file " + pfn).toStdString());
	}
	*/
}

void OsSpecific::SetChown(const QString & pfn)
{
	QFileInfo fi(pfn);
	fi.refresh();
	if (!fi.exists())
		throw std::runtime_error(("Cannot chown. File does not exist " + pfn).toStdString());
	if (fi.ownerId() != 0 || fi.groupId() != 0)
	{
#ifdef Q_OS_MAC
		{
			QStringList args;
			args << "wheel" << pfn;
			ExecAsRoot("/usr/bin/chgrp", args);			// Mac
		}

		{
			QStringList args1;
			args1 << "root" << pfn;
			ExecAsRoot("/usr/sbin/chown", args1);		// Mac
		}
#else
		{
			QStringList args1;
			args1 << "0:0" << pfn;
			ExecAsRoot("/bin/chown", args1);		// ubuntu
		}
#endif
/*
		int r1 = chown(pfn.toStdString().c_str(), 0, 0);
		if (r1 != 0)
		{	// https://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man2/chown.2.html
			// errno.h
			// EPERM		1		// Operation not permitted
			// The effective user ID does not match the owner of the file and the calling process does not have appropriate (i.e., root) privileges.
			throw std::runtime_error(("Cannot chown (err code: " + QString::number(errno) + ") on the file " + pfn).toStdString());
		}
*/
	}
}

void OsSpecific::StartPing(QProcess & pr, const QString & adr)
{
	pr.start(GetCmd(), FormatArgs(adr));
}

int OsSpecific::ExtractPing(QProcess & pr)
{
	int ping = -1;
	QByteArray ba = pr.readAllStandardOutput();
	QString s(ba);
	QStringList out = s.split("\n", QString::SkipEmptyParts);
	if (!out.isEmpty())
	{
		const QString & sp = out.at(out.size() - 1).trimmed();	// last line
#ifndef Q_OS_WIN
		if (sp.indexOf("min/avg/max") > -1)
		{
			int e = sp.indexOf('=');
			int slash = sp.indexOf('/', e +1);
			int sl1 = sp.indexOf('/', slash +1);
			if (sl1 > -1)
			{
				QString sv = sp.mid(slash + 1, sl1 - slash - 1);
				bool ok;
				double d = sv.toDouble(&ok);
				if (ok)
					ping = (int)d;
			}
		}
#else
		int a;
		if ((a = sp.indexOf("Average =")) > -1)
		{
			int e = sp.indexOf('=', a);
			if (e > -1)
			{
				QString val = sp.mid(e + 1, sp.length() - (e + 1 + 2));
				bool ok;
				int p = val.toInt(&ok);
				if (ok)
					ping = p;
			}

		}
#endif
	}
	return ping;
}

QStringList OsSpecific::FormatArgs(const QString & adr)
{
	QStringList args;
	args
#ifndef Q_OS_WIN
		<< "-c" << "1"		// one packet - Mac, Linux
#ifdef Q_OS_LINUX
		<< "-w" << "1"		// 1s deadline - Linux
#endif
#ifdef Q_OS_MAC
		<< "-t" << "1"		// 1s timeout - Mac
#endif
#else
		<< "-n" << "1"		// one packet - Windows
		<< "-w"	<< "1200"	// timeout in ms
#endif
		<< adr
	;
	return args;
}

const QString & OsSpecific::GetCmd()
{
#ifdef  Q_OS_MAC
	static const QString cmd = "/sbin/ping";
#else
#ifdef  Q_OS_LINUX
	static const QString cmd = "/bin/ping";
#else
	static const QString cmd = "ping";
#endif
#endif
	return cmd;
}

int OsSpecific::Ping(const QString & adr)
{
	int n = -1;
#ifdef Q_OS_MAC
	QProcess pr(SjMainWindow::Instance());
	pr.start(GetCmd(), FormatArgs(adr));
	pr.waitForFinished(1500);		// ms
	if (pr.state() == QProcess::NotRunning && pr.exitStatus() == QProcess::NormalExit)
	{
		n = ExtractPing(pr);
	}
	else
	{
		pr.terminate();
	}
#endif
	log::logt("ping " + adr + " " + QString::number(n));

	return n;
}


static const bool gs_isMac =
#ifdef Q_OS_MAC
	true
#else
	false
#endif
	;

static const char * gs_icon = ":/icons/icon-tray.png";
static const char * gs_icon_cross = ":/icons/icon-tray-cross.png";
static const char * gs_icon_cycle = ":/icons/icon-tray-cycle.png";

static const char * gs_icon_light = ":/icons/icon-tray-hover.png";
static const char * gs_icon_cross_light = ":/icons/icon-tray-hover-cross.png";
static const char * gs_icon_cycle_light = ":/icons/icon-tray-hover-cycle.png";

static const char * gs_icon_color = ":/icons/icon-tray-color.png";
static const char * gs_icon_cross_color = ":/icons/icon-tray-color-cross.png";
static const char * gs_icon_cycle_color = ":/icons/icon-tray-color-cycle.png";

const char * OsSpecific::IconDisconnected()
{
	return
#ifdef Q_OS_MAC
	gs_icon_cross
#else
	gs_icon_cross_color
#endif
	;
}

const char * OsSpecific::IconConnecting()
{
	return
#ifdef Q_OS_MAC
	gs_icon_cycle
#else
	gs_icon_cycle_color
#endif
	;
}

const char * OsSpecific::IconConnected()
{
	return
#ifdef Q_OS_MAC
	gs_icon
#else
	gs_icon_color
#endif
	;
}


const char * OsSpecific::IconDisconnected_Selected()
{
	return
#ifdef Q_OS_MAC
	gs_icon_cross_light
#else
	gs_icon_cross_color
#endif
	;
}

const char * OsSpecific::IconConnecting_Selected()
{
	return
#ifdef Q_OS_MAC
	gs_icon_cycle_light
#else
	gs_icon_cycle_color
#endif
	;
}

const char * OsSpecific::IconConnected_Selected()
{
	return
#ifdef Q_OS_MAC
	gs_icon_light
#else
	gs_icon_color
#endif
	;
}

const char * OsSpecific::IsRunningCmd()
{
	return
#ifdef Q_OS_MAC
	"ps -xa | grep open | grep execut | grep Safeju"
#else
	"ps -xa | grep open | grep vpn | grep safej"
#endif
	;
}

#ifdef Q_OS_WIN
static const wchar_t * gs_regpath = L"SYSTEM\\CurrentControlSet\\Services\\Tcpip6\\Parameters";
static const wchar_t * gs_regname = L"DisabledComponents";
struct RegRaii
{
	RegRaii(HKEY & r) : _r(r) {}
	~RegRaii() { RegCloseKey(_r); }
private:
	HKEY _r;
};
#endif

#ifdef Q_OS_OSX
static const QString gs_ns = "/usr/sbin/networksetup";
static const char * gs_wifi = "Wi-Fi";
static const char * gs_ether = "Ethernet";
#endif
void OsSpecific::SetIPv6(bool enable)
{
	bool curr, ok;
	try
	{
		curr = IPv6();
		ok = true;
	}
	catch(...) {}

	bool doit = true;
	if (ok && (curr == enable))
		doit = false;
	if (doit)
	{
		log::logt("Changing IPv6 state to: " + QString(enable ? "enabled" : "disabled"));
#ifdef Q_OS_WIN
		HKEY hKey;
		LONG lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, gs_regpath, 0, KEY_READ | KEY_WRITE, &hKey);
		if (lRes != ERROR_SUCCESS)
		{
			throw std::runtime_error("Cannot open IPv6 reg key for writing");		// another error
		}
		else
		{
			RegRaii ra(hKey);
			DWORD old = 0;
			DWORD val = 0;
			DWORD sz = sizeof(val);
			lRes = ::RegQueryValueExW(hKey, gs_regname, 0, NULL, reinterpret_cast<LPBYTE>(&val), &sz);
			if (ERROR_SUCCESS == lRes)
				old = val;
			if (enable)
				val = old & ( ~((DWORD)0x1));
			else
				val = old | 0x1;
			lRes = ::RegSetValueEx(hKey, gs_regname, 0, REG_DWORD, reinterpret_cast<LPBYTE>(&val), sz);
			if (ERROR_SUCCESS != lRes)
				throw std::runtime_error(("IPv6 disabling failure code: " + QString::number(lRes)).toStdString().c_str());
		}
/*
		static const char * en = "netsh interface ipv6 set teredo client";
		static const char * dis = "netsh interface ipv6 set teredo disabled";
		const char * s = enable ? en : dis;
		int res = QProcess::execute(s);
		log::logt("IPv6 change state command return code: " + QString::number(res));
		if (res != 0)
			throw std::runtime_error(("IPv6 change state failed with return code: " + QString::number(res)).toStdString().c_str());
*/
#else
#ifdef Q_OS_OSX

		static const char * gs_enable_ipv6 = "-setv6automatic";
		static const char * gs_disable_ipv6 = "-setv6off";
		const char * ac = enable ? gs_enable_ipv6 : gs_disable_ipv6;
		{
			QStringList a;
			a << ac << gs_wifi;
			ExecAsRoot(gs_ns, a);
		}
		{
			QStringList a;
			a << ac << gs_ether;
			ExecAsRoot(gs_ns, a);
		}
#else
		// throw std::logic_error("OsSpecific::SetIPv6() Not implemented for this OS");
#endif
#endif	// Q_OS_WIN
	}
}

#ifdef Q_OS_OSX
bool AdapterHasIpv6(const char * adapter)
{
	bool has = true;
	QStringList a;
	a << "-getinfo" << adapter;
	QProcess pr;
	pr.start(gs_ns, a);
	if (pr.waitForFinished(1000))
	{
		QString s = pr.readAllStandardOutput();
		if (s.indexOf("IPv6:") > -1)
		{
			has = true;
			if (s.indexOf("IPv6: Off") > -1)
				has = false;
		}
		else
		{
			has = false;
		}
	}
	return has;
}
#endif

bool OsSpecific::IPv6()
{
	bool on = true;
#ifdef Q_OS_WIN
	HKEY hKey;
	LONG lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, gs_regpath, 0, KEY_READ, &hKey);
	if (lRes != ERROR_FILE_NOT_FOUND)
	{
		if (lRes == ERROR_SUCCESS)
		{
			RegRaii ra(hKey);
			DWORD val;
			DWORD sz = sizeof(val);
			lRes = ::RegQueryValueExW(hKey, gs_regname, 0, NULL, reinterpret_cast<LPBYTE>(&val), &sz);
			if (lRes != ERROR_FILE_NOT_FOUND)
			{
				if (lRes == ERROR_SUCCESS)
				{
					if ( (val & 0x1) == 0x1)	// 0x01 to disable IPv6 on all tunnel interfaces https://support.microsoft.com/en-us/kb/929852
						on = false;
				}
				else
				{
					throw std::runtime_error("Cannot read IPv6 reg key value");		// another error
				}
			}
		}
		else
		{
			throw std::runtime_error("Cannot read IPv6 reg key");		// another error
		}
	}
/*	static const char * s0 = "netsh interface ipv6 show teredo";
	QProcess p;
	p.start(s0);
	if (p.waitForFinished(3000))	// 3s
	{	// finished
		QString ts(p.readAllStandardOutput());
		int state = ts.indexOf("State");
		if (state > -1)
		{
			if (ts.indexOf("offline", state) > -1)
				on = false;
		}
	}
	else
	{	// timeout
		if (QProcess::NotRunning != p.state() )
			p.terminate();
		throw std::runtime_error("Failed to execute process to get IPv6 state");
	}
*/
#else
#ifdef Q_OS_OSX
	on =	AdapterHasIpv6(gs_wifi);
	if (!on)
		on =	AdapterHasIpv6(gs_ether);
#else
	//throw std::logic_error("OsSpecific::IPv6() Not implemented for this OS");
#endif
#endif	// Q_OS_WIN
	return on;
}

void OsSpecific::EnableTap()
{
#ifdef Q_OS_WIN
	// https://airvpn.org/topic/12599-“air-vpn-hack-executed’/#entry21338

	// https://msdn.microsoft.com/en-us/library/windows/desktop/aa365915(v=vs.85).aspx
	ULONG sz = 0;
	ULONG fl = 0
		| GAA_FLAG_SKIP_DNS_SERVER
		| GAA_FLAG_INCLUDE_TUNNEL_BINDINGORDER
		| GAA_FLAG_INCLUDE_PREFIX
		| GAA_FLAG_INCLUDE_ALL_INTERFACES
	;
	ULONG r = GetAdaptersAddresses(0, fl, 0, NULL, &sz);
	if (ERROR_BUFFER_OVERFLOW != r)
		return;
	std::vector<byte> v;
	v.assign(sz, 0);
	IP_ADAPTER_ADDRESSES * p = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(&v[0]);
	r = GetAdaptersAddresses(0, fl, 0, p, &sz);
	if (NO_ERROR != r)
	{
		log::logt("Cannot enumerate adapters, error code: " + QString::number(r));
	}
	else
	{
		for (; p != NULL; p = p->Next)
		{
			QString s = QString::fromWCharArray(p->Description);
			if (s.indexOf("TAP-Windows") > -1)
			{
				std::wstring prog = L"netsh";
				std::wstring wa = L"interface set interface name=\"" + std::wstring(p->FriendlyName) + L"\" admin=enabled";
				log::logt("ShellExecute '" + QString::fromStdWString(prog) + "' , Args: '" + QString::fromStdWString(wa) + "'");
				HINSTANCE hi = ShellExecute(NULL, NULL, prog.c_str(), (LPWSTR)wa.c_str(), NULL, SW_HIDE);	// already admin
				if ((int)hi <= 32)
					log::logt("Cannot ShellExecute hi = " + QString::number((int)hi) + " err code = " + QString::number(GetLastError()));
				else
					log::logt("ShellExecute OK");
			}
		}
	}
#endif	// Q_OS_WIN
}

#ifdef Q_OS_LINUX

static const QString gs_desktop = 
	"[Desktop Entry]\n"
	"Type=Application\n"
	"Name=Safejumper\n"
	"Exec=/opt/safejumper/safejumper.sh\n"
	"Icon=/usr/share/icons/hicolor/64x64/apps/safejumper.png\n"
	"Comment=OpenVPN client for proxy.sh\n"
	"X-GNOME-Autostart-enabled=true\n"
	;

// f - opened for read-write
void delete_startup(QFile & f)
{
	int ncount = gs_desktop.count('\n');
	int n3 = 0;
	for (int k = 0; k < 3; ++k)
		n3 = gs_desktop.indexOf('\n', n3 + 1);
	if (n3 < 0)
		return;

	QString s(f.readAll());
	
	QString begining = gs_desktop.mid(0, n3);
	int p = s.indexOf(begining);
	if (p < 0)
		return;
	
	int lastn = p;
	for (int k = 0; lastn > -1 && k < ncount; ++k)
		lastn = gs_desktop.indexOf('\n', lastn + 1);
	if (lastn < 0)
	{
		log::logt("Openned .desktop file but cannot find proper " + QString::number(ncount) + " lines");
		return;	// err
	}
	
	QString remains = s.mid(0, p);
	remains += s.mid(lastn + 1);
	
	QByteArray out = remains.toLatin1();
	f.resize(out.length());
	f.write(out);
	f.flush();
}
#endif

void OsSpecific::SetStartup(bool b)
{
#ifdef Q_OS_WIN
	{
		QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
		static const char * keyname = "proxy_sh";
		if (!b)
		{
			settings.remove(keyname);
		}
		else
		{
			QString val = "\"" + QCoreApplication::applicationFilePath() + "\"";
			val.replace("/","\\");
			settings.setValue(keyname, val);
		}
	}
#endif	// Q_OS_WIN

#ifdef Q_OS_LINUX
	{
		QString dir = QDir::homePath() + "/.config/autostart";
		QString pfn = dir + "/.desktop";
		if (b)
		{
			QDir d;
			if (d.mkpath(dir))
			{
				QFile f(pfn);
				if (f.exists())
				{
					f.open(QIODevice::ReadWrite);
					delete_startup(f);
				}
				else
				{
					f.open(QIODevice::Append);
				}
				f.write(gs_desktop.toLatin1());
				f.flush();
				f.close();
			}
		}
		else
		{
			QFile f(pfn);
			if (f.exists())
			{
				f.open(QIODevice::ReadWrite);
				delete_startup(f);
				f.flush();
				f.close();
			}
		}
	}
#endif	// Q_OS_LINUX

#ifdef Q_OS_OSX
	QString dir = QDir::homePath() + "/Library/LaunchAgents";
	QString pfn = dir + "/sh.proxy.safejumper.plist";
	QFile pa(pfn);
	if (pa.exists())
	{
		if (!pa.remove())
			log::logt("Cannot delete startup file '"+ pfn + "'");
	}
	if (b)
	{
		QDir d(dir);
		if (!d.exists())
		{
			d.mkpath(dir);
		}
		if (!pa.open(QIODevice::WriteOnly))
		{
			log::logt("Cannot open startup file '"+ pfn + "' for writing");
		}
		else
		{

			QString s =
				"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
				"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
				"<plist version=\"1.0\">\n"
				"<dict>\n"
					"<key>Label</key>\n"
					"<string>sh.proxy.safejumper</string>\n"

					"<key>LimitLoadToSessionType</key>\n"
					"<string>Aqua</string>\n"
					"<key>OnDemand</key>\n"
					"<true/>\n"

					"<key>Program</key>\n"
					"<string>"
				;

			s +=
				QCoreApplication::applicationFilePath() + "</string>\n"

				//	  "<key>ProgramArguments</key>\n"
				//	  "<array>\n"
				//	  "<string>";
				// <string>arguments_here</string>\n
				//	"</array>\n"

					"<key>KeepAlive</key>\n"
					"<false/>\n"
					"<key>RunAtLoad</key>\n"
					"<true/>\n"
				"</dict>\n"
				"</plist>\n"
				;
			pa.write(s.toLatin1());
		}
	}
#endif	// Q_OS_OSX

}

void OsSpecific::NetDown()
{
	try
	{
#ifndef Q_OS_WIN
	SetRights();
	log::logt("NetDown()");
	QString ss = RunFastCmd(PathHelper::Instance()->NetDownPfn());
	_netdown = true;
	if (!ss.isEmpty())
		log::logt(ss);
#else	// Q_OS_WIN
	// https://airvpn.org/topic/12599-“air-vpn-hack-executed’/#entry21338

	// https://msdn.microsoft.com/en-us/library/windows/desktop/aa365915(v=vs.85).aspx
	ULONG sz = 0;
	ULONG fl = 0
		| GAA_FLAG_SKIP_DNS_SERVER
		| GAA_FLAG_INCLUDE_TUNNEL_BINDINGORDER
		| GAA_FLAG_INCLUDE_PREFIX
		| GAA_FLAG_INCLUDE_ALL_INTERFACES
	;
	ULONG r = GetAdaptersAddresses(0, fl, 0, NULL, &sz);
	if (ERROR_BUFFER_OVERFLOW != r)
		return;
	std::vector<byte> v;
	v.assign(sz, 0);
	IP_ADAPTER_ADDRESSES * p = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(&v[0]);
	r = GetAdaptersAddresses(0, fl, 0, p, &sz);
	if (NO_ERROR != r)
	{
		log::logt("Cannot enumerate adapters, error code: " + QString::number(r));
	}
	else
	{
		for (int k = 1; p != NULL && k < 5; p = p->Next, ++k)		// adpters in order - so try first 4
		{
			QString s = QString::fromWCharArray(p->Description);
			if (s.indexOf("TAP-Windows") > -1) continue;
			if (s.indexOf("WAN Miniport") > -1) continue;
			if (s.indexOf("Kernel Debug") > -1) continue;
			if (s.indexOf("ISATAP") > -1) continue;
			if (s.indexOf("Tunelling") > -1) continue;
			QString name = QString::fromWCharArray(p->FriendlyName);
			if (name.indexOf("isatap") > -1) continue;
			if (name.indexOf("isatap") > -1) continue;


			{
				std::wstring prog = L"netsh";
				std::wstring wa = L"interface set interface name=\"" + std::wstring(p->FriendlyName) + L"\" admin=disable";
				log::logt("ShellExecute '" + QString::fromStdWString(prog) + "' , Args: '" + QString::fromStdWString(wa) + "'");
				HINSTANCE hi = ShellExecute(NULL, NULL, prog.c_str(), (LPWSTR)wa.c_str(), NULL, SW_HIDE);	// already admin
				if ((int)hi <= 32)
					log::logt("Cannot ShellExecute hi = " + QString::number((int)hi) + " err code = " + QString::number(GetLastError()));
				else
				{
					log::logt("ShellExecute OK");
					_netdown = true;
				}
			}
		}
	}
#endif	// Q_OS_WIN
	}
	catch(std::exception & ex)
	{
		log::logt(ex.what());
	}
}

QString OsSpecific::RunFastCmd(const QString & cmd, uint16_t ms /* = 500 */)
{
	return RunFastCmd(cmd.toStdString().c_str(), ms);
}

QString OsSpecific::RunFastCmd(const char * cmd, uint16_t ms /* = 500 */)
{
	std::auto_ptr<QProcess> pr(new QProcess());
	pr->start(cmd);
	if (!pr->waitForFinished(ms))
	{
		QString s1(pr->readAllStandardError());
		log::logt("RunFastCmd(): Error: " + s1);
		if (QProcess::NotRunning != pr->state())
		{
			pr->terminate();
			pr->kill();
		}
	}
	QString s0(pr->readAllStandardOutput());
	pr.release()->deleteLater();
	return s0;
}

bool OsSpecific::HasInsecureWifi()
{
	bool has = false;

#ifdef Q_OS_OSX
	std::vector<std::string> wifi_devices;
	QString s0 = RunFastCmd("/usr/sbin/networksetup -listallhardwareports");
	QStringList sl0 = s0.split(QString("Hardware Port:"), QString::SkipEmptyParts);
	for (int k = 0; k < sl0.length(); ++k)
	{
		const QString & rs = sl0.at(k);
		if (rs.contains("Wi-Fi", Qt::CaseInsensitive) || rs.contains("Airport", Qt::CaseInsensitive))		// since 10.7 changed to Wi-Fi
		{
			static const QString s_device = "Device: ";
			int p = rs.indexOf(s_device, 0, Qt::CaseInsensitive);
			if (p > -1)
			{
				int p1 = rs.indexOf('\n', p + s_device.length(), Qt::CaseInsensitive);
				if (p1 > -1)
				{
					QString dev = rs.mid(p + s_device.length(), p1 - p - s_device.length());
					dev = dev.trimmed();
					if (dev.length() > 1)
						wifi_devices.push_back(dev.toStdString());
				}
			}
		}

	}

	bool on = false;
	// 	get protocol for each wi-fi device
	for (size_t k = 0; !on && k < wifi_devices.size(); ++k)
	{
		std::string s = "/usr/sbin/networksetup -getairportpower " + wifi_devices.at(k);
		QString s1 = RunFastCmd(s.c_str());
		if (s1.contains("on", Qt::CaseInsensitive))
			on = true;
	}

	if (on)
	{
		QString s1 = RunFastCmd("/System/Library/PrivateFrameworks/Apple80211.framework/Versions/Current/Resources/airport -I");
		int p1 = s1.indexOf("link auth:", 0, Qt::CaseInsensitive);
		if (p1 > -1)
		{
			int p2 = s1.indexOf('\n', p1 + 1, Qt::CaseInsensitive);
			QString s2 = s1.mid(p1, p2 - p1 + 1);
			if (!s2.contains("wpa2", Qt::CaseInsensitive))
			{
				has = true;
				log::logt("Detected unsecure Wi-Fi with " + s2.trimmed());
			}
		}
	}
#endif	// Q_OS_OSX

	return has;
}

void OsSpecific::FixDnsLeak()
{
#ifdef Q_OS_WIN
	QSysInfo::WinVersion v = QSysInfo::windowsVersion();
	if (v >= QSysInfo::WV_WINDOWS8)
	{
		QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows NT\\DNSClient", QSettings::NativeFormat);
		QVariant val = 1;
		settings.setValue("DisableSmartNameResolution", val);
	}
#endif
}

static std::auto_ptr<QProcess> _obfs;
void OsSpecific::RunObfs()
{
	if (!IsObfsInstalled())
		InstallObfs();

#ifdef Q_OS_MAC
	if (!IsObfsRunning())
	{
		_obfs.reset(new QProcess());
		_obfs->start("/usr/local/bin/obfsproxy obfs2 socks 127.0.0.1:1050");
		QThread::msleep(200);
	}
#endif	// 	Q_OS_MAC

}

void OsSpecific::InstallObfs()
{
	;
}

bool OsSpecific::IsObfsInstalled()
{
	return true;
}

bool OsSpecific::IsObfsRunning()
{
	return true;
	bool b = false;
//	if (_obfs.get() != NULL)
	{
		static const char * q = "ps aux | grep obfs | grep pro";
		QString s = RunFastCmd(q);
		if (!s.isEmpty())
			b = true;
	}
	return b;
}


