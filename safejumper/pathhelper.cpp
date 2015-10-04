#include "pathhelper.h"

#include <QDir>
#include <QApplication>
#include <QProcess>

//#include "log.h"	// TODO: -0 remove

extern QApplication * g_pTheApp;

std::auto_ptr<PathHelper> PathHelper::_inst;
PathHelper * PathHelper::Instance()
{
	if (!_inst.get())
		_inst.reset(new PathHelper());
	return _inst.get();
}

PathHelper::PathHelper()
{}

PathHelper::~PathHelper()
{}

QString PathHelper::OpenvpnPathfilename()
{
	if (_openvpn.isEmpty())
	{
#ifdef Q_OS_MAC
		_openvpn = ResourcesPath() + OvRelativePfn();
#else
#ifdef Q_OS_WIN
		//_openvpn = "c:\\Program Files\\OpenVPN\\bin\\openvpn.exe";
		_openvpn = "c:/Program Files/OpenVPN/bin/openvpn.exe";
#else	// Q_OS_LINUX
		_openvpn = "/opt/safejumper/openvpn";
		QProcess pr;
		pr.start("which openvpn");
		pr.waitForFinished(3000);	// 3s
		bool ok = true;
		if (pr.state() != QProcess::NotRunning)
		{
			pr.terminate();
			ok = false;
		}
		if (pr.exitStatus() != QProcess::NormalExit)
			ok = false;
		if (pr.exitCode() != 0)
			ok = false;
		QString stdout = pr.readAllStandardOutput();
		QString stderr = pr.readAllStandardError();	
		if (!stderr.isEmpty())
			ok = false;
		if (ok)
		{
			if (!stdout.trimmed().isEmpty())
				_openvpn = stdout.trimmed();
			else
				_openvpn = "/opt/safejumper/openvpn";		// default location of onboard file
		}
QString sb("Linux OpenVpn binary is: " + _openvpn);
printf("%s", sb.toStdString().c_str());

#endif
#endif
	}
	return _openvpn;
}

QString PathHelper::OvRelativePfn()
{
#ifdef Q_OS_MAC
	return "/openvpn/openvpn-2.3.2/openvpn-executable";
#endif
#ifdef Q_OS_WIN
	return "/openvpn-proxysh.exe";
#endif
}

QString PathHelper::ResourcesPath()
{
	QString s = ".";
#ifdef Q_OS_MAC
	QDir d(g_pTheApp->applicationDirPath());
	d.cdUp();
	d.cd("Resources");
	s = d.canonicalPath();
#else
	s = QApplication::applicationDirPath();
#endif
	return s;
}

QString PathHelper::ContentPath()
{
	QString s = ".";
#ifdef Q_OS_MAC
	QDir d(g_pTheApp->applicationDirPath());
	d.cdUp();
	s = d.canonicalPath();
#endif
	return s;
}

QString PathHelper::OpenvpnWorkdir()
{
	// TODO: -1
	return "/tmp";
}

QString PathHelper::OpenvpnLogPfn()
{
#ifndef Q_OS_WIN
	return "/tmp/safejumper-openvpn.log";
#else
	return QDir::tempPath() + "/safejumper-openvpn.log";
#endif // Q_OS_WIN
}

QString PathHelper::OpenvpnConfigPfn()
{
#ifndef Q_OS_WIN
	return "/tmp/safejumper-openvpn.ovpn";
#else
	return QDir::tempPath() + "/safejumper-openvpn.ovpn";
#endif // Q_OS_WIN
}

QString PathHelper::ProxyshCaCert()
{
	return ResourcesPath() + "/proxysh.crt";
}

QString PathHelper::UpScriptPfn()
{
	return ResourcesPath() + "/client.up.safejumper.sh";
}

QString PathHelper::DownScriptPfn()
{
	return ResourcesPath() + "/client.down.safejumper.sh";
}

QString PathHelper::ScriptPath()
{
	return ResourcesPath();
}

QString PathHelper::LauncherPfn()
{
	return ResourcesPath() + "/launchopenvpn";
}

QString PathHelper::LogPfn()
{
#ifndef Q_OS_WIN
	return "/tmp/safejumper-debug.log";
#else
	return QDir::tempPath() + "/safejumper-debug.log";
#endif // Q_OS_WIN
}


