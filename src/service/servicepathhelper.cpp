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

#include "servicepathhelper.h"

#include "servicelog.h"

#include <QDir>
#include <QCoreApplication>
#include <QProcess>

#ifdef Q_OS_LINUX
#include "osspecific.h"
#endif

extern QCoreApplication * g_pTheApp;

#ifdef Q_OS_LINUX
bool detectionDone = false;
#endif

std::auto_ptr<ServicePathHelper> ServicePathHelper::_inst;
ServicePathHelper * ServicePathHelper::Instance()
{
    if (!_inst.get())
        _inst.reset(new ServicePathHelper());
#ifdef Q_OS_LINUX
    if (!detectionDone) {
        detectionDone = true;
        _inst->mUseSystemdResolver = false;
        QString result = OsSpecific::instance()->runCommandFast("/opt/safejumper/detectresolve.sh");
        if (result.isEmpty()) {
            Log::serviceLog("script to check if resolve1 service is registered did not run");
        } else {
            _inst->mUseSystemdResolver = (result.trimmed() == "1");
            Log::serviceLog(QString("dbus reply to check if resolve1 service is registered gave result %1").arg(_inst->mUseSystemdResolver));
        }
    }
#endif
    return _inst.get();
}

bool ServicePathHelper::exists()
{
    return (_inst.get() != NULL);
}

void ServicePathHelper::cleanup()
{
    if (_inst.get() != NULL)
        delete _inst.release();
}

ServicePathHelper::ServicePathHelper()
{
    QDir dir(tempPath());
    if (!dir.exists())
        dir.mkpath(tempPath());
}

ServicePathHelper::~ServicePathHelper()
{}

QString ServicePathHelper::openvpnFilename()
{
#ifdef Q_OS_DARWIN
    return resourcesPath() + openvpnRelativeFilename();
#else
#ifdef Q_OS_WIN
    return resourcesPath() + "/OpenVPN/bin/openvpn.exe";
#else	// Q_OS_LINUX
    return "/opt/safejumper/openvpn";
#endif
#endif
}

#ifdef Q_OS_DARWIN
QString ServicePathHelper::openvpnRelativeFilename()
{
    return "/openvpn/openvpn-executable";
}
#endif

QString ServicePathHelper::serviceLogPath()
{
#ifdef Q_OS_DARWIN
    return "/Library/Logs";
#else
    return resourcesPath();
#endif
}

QString ServicePathHelper::resourcesPath()
{
#ifdef Q_OS_DARWIN
    return "/Applications/Safejumper.app/Contents/Resources";
#else
    return QCoreApplication::applicationDirPath();
#endif
}

#ifdef Q_OS_DARWIN
QString ServicePathHelper::openvpnRunningScriptFilename()
{
    return resourcesPath() + "/openvpnRunning.sh";
}
#endif

QString ServicePathHelper::tempPath()
{
    return QDir::homePath() + "/.safejumper/";
}

QString ServicePathHelper::openvpnLogFilename()
{
    return serviceLogPath() + "/safejumper-openvpn.log";
}

QString ServicePathHelper::openvpnConfigFilename()
{
    return serviceLogPath() + "/safejumper-openvpn.ovpn";
}

QString ServicePathHelper::proxyshCaCertFilename()
{
    return resourcesPath() + "/proxysh.crt";
}

QString ServicePathHelper::upScriptFilename()
{
#ifdef Q_OS_LINUX
    if (!mUseSystemdResolver)
#endif
        return resourcesPath() + "/client.up.safejumper.sh";
#ifdef Q_OS_LINUX
    else
        return resourcesPath() + "/update-systemd-resolved";
#endif
}

QString ServicePathHelper::downScriptFilename()
{
#ifdef Q_OS_LINUX
    if (!mUseSystemdResolver)
#endif
        return resourcesPath() + "/client.down.safejumper.sh";
#ifdef Q_OS_LINUX
    else
        return resourcesPath() + "/update-systemd-resolved";
#endif
}

QString ServicePathHelper::netDownFilename()
{
    return resourcesPath() + "/netdown";
}

QString ServicePathHelper::launchopenvpnFilename()
{
    return resourcesPath() + "/launchopenvpn";
}

QString ServicePathHelper::obfsproxyFilename()
{
#ifdef Q_OS_DARWIN
    return resourcesPath() + "/env/bin/obfsproxy";
#else
#ifdef Q_OS_LINUX
    return "/opt/safejumper/env/bin/obfsproxy";
#else		// Win
    return         "cmd /k c:\\python27\\Scripts\\obfsproxy.exe";
#endif	// linux
#endif	// 	Q_OS_DARWIN
}

QString ServicePathHelper::obfsproxyLogFilename()
{
    return serviceLogPath() + "/safejumper-obfsproxy.log";
}

QString ServicePathHelper::installObfsproxyFilename()
{
    return resourcesPath() + "/installobfsproxy.sh";
}

QString ServicePathHelper::safejumperServiceLogFilename()
{
#ifdef Q_OS_WIN
    return resourcesPath() + "/safejumper-service.log";
#else
    return "/tmp/safejumper-service.log";
#endif
}

QString ServicePathHelper::safejumperLogFilename()
{
    return tempPath() + "safejumper-debug.log";
}
