#include "pathhelper.h"

#include <QDir>
#include <QCoreApplication>
#include <QProcess>

extern QCoreApplication * g_pTheApp;

std::auto_ptr<PathHelper> PathHelper::_inst;
PathHelper * PathHelper::Instance()
{
    if (!_inst.get())
        _inst.reset(new PathHelper());
    return _inst.get();
}

bool PathHelper::exists()
{
    return (_inst.get() != NULL);
}

void PathHelper::cleanup()
{
    if (_inst.get() != NULL)
        delete _inst.release();
}

PathHelper::PathHelper()
{}

PathHelper::~PathHelper()
{}

QString PathHelper::openvpnFilename()
{
#ifdef Q_OS_MAC
    return resourcesPath() + openvpnRelativeFilename();
#else
#ifdef Q_OS_WIN
    return "c:/Program Files/OpenVPN/bin/openvpn.exe";
#else	// Q_OS_LINUX
    return "/opt/safejumper/openvpn";
#endif
#endif
}

QString PathHelper::openvpnRelativeFilename()
{
#ifdef Q_OS_MAC
    return "/openvpn/openvpn-2.3.13/openvpn-executable";
#endif
}

QString PathHelper::resourcesPath()
{
#ifdef Q_OS_MAC
    QDir d(g_pTheApp->applicationDirPath());
    d.cdUp();
    d.cd("Resources");
    return d.canonicalPath();
#else
    return QCoreApplication::applicationDirPath();
#endif
}

QString PathHelper::openvpnLogFilename()
{
#ifndef Q_OS_WIN
    return "/tmp/safejumper-openvpn.log";
#else
    return QDir::tempPath() + "/safejumper-openvpn.log";
#endif // Q_OS_WIN
}

QString PathHelper::openvpnConfigFilename()
{
#ifndef Q_OS_WIN
    return "/tmp/safejumper-openvpn.ovpn";
#else
    return QDir::tempPath() + "/safejumper-openvpn.ovpn";
#endif // Q_OS_WIN
}

QString PathHelper::proxyshCaCertFilename()
{
    return resourcesPath() + "/proxysh.crt";
}

QString PathHelper::upScriptFilename()
{
    return resourcesPath() + "/client.up.safejumper.sh";
}

QString PathHelper::downScriptFilename()
{
    return resourcesPath() + "/client.down.safejumper.sh";
}

QString PathHelper::netDownFilename()
{
    return resourcesPath() + "/netdown";
}

QString PathHelper::launchopenvpnFilename()
{
    return resourcesPath() + "/launchopenvpn";
}

QString PathHelper::obfsproxyFilename()
{
#ifdef Q_OS_MAC
    return resourcesPath() + "/env/bin/obfsproxy";
#else
#ifdef Q_OS_LINUX
    return "/usr/local/bin/obfsproxy";
#else		// Win
    return "";
#endif	// linux
#endif	// 	Q_OS_MAC
}

QString PathHelper::installObfsproxyFilename()
{
    return resourcesPath() + "/installobfsproxy.sh";
}

QString PathHelper::safejumperLogFilename()
{
#ifndef Q_OS_WIN
    return "/tmp/safejumper-debug.log";
#else
    return QDir::tempPath() + "/safejumper-debug.log";
#endif // Q_OS_WIN
}


