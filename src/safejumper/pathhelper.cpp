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
{
    QDir dir(tempPath());
    if (!dir.exists())
        dir.mkpath(tempPath());
}

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
#else
    return "";
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

QString PathHelper::tempPath()
{
#ifdef Q_OS_DARWIN
    return QDir::homePath() + "/.safejumper/";
#else
    return "/tmp/";
#ifdef Q_OS_WIN
    return QDir::tempPath();
#endif
#endif
}

QString PathHelper::openvpnLogFilename()
{
    return tempPath() + "safejumper-openvpn.log";
}

QString PathHelper::openvpnConfigFilename()
{
    return tempPath() + "safejumper-openvpn.ovpn";
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
    return tempPath() + "safejumper-debug.log";
}


