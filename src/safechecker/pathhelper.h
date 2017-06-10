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

// This PathHelper is for Safechecker so it uses different paths than Safejumper
#ifndef PATHHELPER_H
#define PATHHELPER_H

#include <memory>

#include <QString>

class PathHelper
{
public:
    static bool exists();
    static PathHelper * Instance();
    static void cleanup();

    ~PathHelper();

    QString openvpnFilename();
#ifdef Q_OS_DARWIN
    QString openvpnRelativeFilename();
#endif
    QString openvpnLogFilename();
    QString openvpnConfigFilename();
    QString proxyshCaCertFilename();
    QString upScriptFilename();
    QString downScriptFilename();
    QString launchopenvpnFilename();
    QString obfsproxyFilename();
    QString obfsproxyLogFilename();
    QString installObfsproxyFilename();
    QString netDownFilename();
    QString safejumperLogFilename(); // "/tmp/Safejumper-debug.log"

    QString resourcesPath();

#ifdef Q_OS_DARWIN
    // This script never changes, so why keep writing it to a temp file, just run it.
    QString openvpnRunningScriptFilename();
#endif
private:
    QString tempPath(); // Where to keep config file, logs etc.
    PathHelper();
    static std::auto_ptr<PathHelper> _inst;
};

#endif // PATHHELPER_H

