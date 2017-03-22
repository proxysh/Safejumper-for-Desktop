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

#ifdef Q_OS_DARWIN
#include <Security/Authorization.h>
#include <Security/AuthorizationTags.h>
#include <stdio.h>
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
#include "loginwindow.h"
#include "wndmanager.h"
#include "pathhelper.h"
#include "openvpnmanager.h"


std::auto_ptr<OsSpecific> OsSpecific::mInstance;
bool OsSpecific::exists()
{
    return (mInstance.get() != NULL);
}

OsSpecific * OsSpecific::instance()
{
    if (!mInstance.get())
        mInstance.reset(new OsSpecific());
    return mInstance.get();
}

void OsSpecific::cleanup()
{
    if (mInstance.get() != NULL) delete mInstance.release();
}

OsSpecific::OsSpecific()
    : mNetDown(false)
{}

bool _auth(false);
#ifdef Q_OS_DARWIN
AuthorizationRef _AuthorizationRef;
// throw std::exception on error
AuthorizationRef & getMacAuthorization()
{
    if (!_auth) {
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

void OsSpecific::setOwnerRoot(const QString & pfn)
{
    // check if bit set already
    if (!isOwnerRoot(pfn)) {
#ifdef Q_OS_WIN
#error SetOwnerRoot() Not implemented
        throw std::runtime_error("SetOwnerRoot() Not implemented");
#endif
        AuthorizationRef & ra = getMacAuthorization();
//		QStringList args;
//		args << "u+s" << pfn;
//		execAsRoot("chmod", args);

        //int	chmod(const char *, mode_t) __DARWIN_ALIAS(chmod);
        int r1 = chmod(pfn.toLatin1(), 04555);
        if (r1 != 0) {
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
                execAsRoot("ls", args2);
            }
        */
        try {
            QStringList args;
            args << "root" << pfn;
            execAsRoot("/usr/sbin/chown", args);


            // additional files
        } catch(std::exception & ex) {
            // do not propagate - allow different method of OpenVPN execution regardless this property change
            log::logt(QString("Cannot set owner of ") + pfn + "err = " + ex.what());
        }
    }
}
#endif

OsSpecific::~OsSpecific()
{
    if (_auth) {
#ifdef Q_OS_DARWIN
        OSStatus res = AuthorizationFree(_AuthorizationRef, kAuthorizationFlagDefaults);
#endif
        _auth = false;
    }
    if (mObfsproxy.get()) {
        mObfsproxy->terminate();
        // TODO: -2 gracefully stop
    }
}

bool OsSpecific::isOwnerRoot(const QString & pfn)
{
    QFileInfo fi(pfn);
    return fi.exists() && fi.ownerId() == 0;
}

void OsSpecific::releaseRights()
{
    if (_auth) {
#ifdef Q_OS_DARWIN
        AuthorizationFree(_AuthorizationRef, kAuthorizationFlagDefaults);
#endif
        _auth = false;
    }
}

#ifdef Q_OS_DARWIN
static void execAsRootMac(const QString & cmd, char * const * argv)
{
    //execute
    AuthorizationRef & ra = getMacAuthorization();
    OSStatus res = AuthorizationExecuteWithPrivileges(ra,
                   cmd.toStdString().c_str()	//myToolPath
                   , kAuthorizationFlagDefaults,
                   argv									//myArguments
                   , NULL);							// pipes

    // handle misterious errors with auth service
    for (int k = 1; k < 6 && errAuthorizationToolEnvironmentError == res; ++k) {	// 1500ms  total
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
}
#endif	// Q_OS_DARWIN

#ifdef Q_OS_LINUX
static void execAsRootLinux(const QString & cmd, const QStringList & args)
{
    QStringList args1;
    args1 << cmd << args;
    int re = QProcess::execute("pkexec", args1);
    if (re != 0) {
        QString s1 = "Unable to run command as root\n Result code is " + QString::number(re);
        if (re == 126)
            s1 = "Unable to run command as root.\n User declined auth.";
        s1 += "\nCommand: " + cmd;
        throw std::runtime_error(s1.toStdString());
    }
}
#endif // Q_OS_LINUX

void OsSpecific::execAsRoot(const QString & cmd, const QStringList & args)
{
    QString m = "Executing as root. Cmd = " + cmd + " args = ";
    m += args.join(' ');
    log::logt(m);

#ifdef Q_OS_DARWIN
    std::vector<const char *> argv1;
    std::vector<std::string> argv_store;
    argv1.reserve(args.size() + 1);
    argv_store.reserve(args.size());
    for (int k = 0; k < args.size(); ++k) {
        argv_store.push_back(args.at(k).toStdString());
        std::string & ts = argv_store.at(k);
        argv1.push_back(ts.c_str());
    }
    argv1.push_back(NULL);

    execAsRootMac(cmd, (char * const *)&argv1[0]);
#else		// Q_OS_DARWIN
#ifdef Q_OS_LINUX
    execAsRootLinux(cmd, args);
#else
    throw std::runtime_error("non Mac, non Linux exec as root");
#endif	//Q_OS_LINUX
#endif	// Q_OS_DARWIN
    // bring our window back to front after OS password dialog
    WndManager::Instance()->ToFront();

    log::logt("execAsRoot() success");
}

void OsSpecific::setRights()
{
    // Qt uses permissions in HEX while OS's in 8-based !


    // will ask for elevated rights inside
#ifdef Q_OS_DARWIN

    setChmod("0744", PathHelper::Instance()->upScriptFilename());
    setChown(PathHelper::Instance()->upScriptFilename());
    setChmod("0744", PathHelper::Instance()->downScriptFilename());
    setChown(PathHelper::Instance()->downScriptFilename());

    setChmod("0755", PathHelper::Instance()->openvpnFilename());
    setChown(PathHelper::Instance()->openvpnFilename());

    setChmod("04555", PathHelper::Instance()->netDownFilename());
    setChown(PathHelper::Instance()->netDownFilename());

    setChmod("04555", PathHelper::Instance()->launchopenvpnFilename());
    setChown(PathHelper::Instance()->launchopenvpnFilename());

    system(QString("touch %1").arg(PathHelper::Instance()->openvpnLogFilename()).toStdString().c_str());
    setChmod("777", PathHelper::Instance()->openvpnLogFilename());
#endif		// Q_OS_DARWIN

#ifdef Q_OS_LINUX
    setChown(PathHelper::Instance()->launchopenvpnFilename());
    setChmod("777", PathHelper::Instance()->launchopenvpnFilename());				// odrer is important

    setChown(PathHelper::Instance()->netDownFilename());
    setChmod("777", PathHelper::Instance()->netDownFilename());
#endif

    //ReleaseRights();

    QThread::msleep(200);		// HACK: -1 wait till file system changes become effective
}

void OsSpecific::setChmod(const char * sflags, const QString & pfn)
{
    struct stat st;		// QFileInfo fails often
    if (0 != stat(pfn.toStdString().c_str(), &st))
        throw std::runtime_error(("Cannot read flags on file " + pfn).toStdString());

    bool ok;
    QString qs(sflags);
    //unsigned flags16 = qs.toInt(&ok, 16);
    //if (!ok) throw std::runtime_error((QString("Internal error: Cannot atoi ") + sflags).toStdString());
    unsigned flags8 = qs.toInt(&ok, 8);
    if (!ok)
        throw std::runtime_error((QString("Internal error: Cannot atoi ") + sflags).toStdString());

    if ((st.st_mode & flags8) != flags8) {
        QStringList args1;
        args1 << sflags << pfn;
        execAsRoot("/bin/chmod", args1);				// mac, ubuntu

//		QStringList a;		// HACK: -1 wait till file system changes become effective
//		a << pfn;
//		//QProcess::execute("/usr/bin/touch", a);
//		execAsRoot("/usr/bin/touch", a);

        // HACK: wait till file system changes become effective
        int r2 = stat(pfn.toStdString().c_str(), &st);
        for (int k = 1; k < 6 && 0 == r2 && ((st.st_mode & flags8) != flags8) ; ++k) {
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

void OsSpecific::setChown(const QString & pfn)
{
    QFileInfo fi(pfn);
    fi.refresh();
    if (!fi.exists())
        throw std::runtime_error(("Cannot chown. File does not exist " + pfn).toStdString());
    if (fi.ownerId() != 0 || fi.groupId() != 0) {
#ifdef Q_OS_DARWIN
        {
            QStringList args;
            args << "wheel" << pfn;
            execAsRoot("/usr/bin/chgrp", args);			// Mac
        }

        {
            QStringList args1;
            args1 << "root" << pfn;
            execAsRoot("/usr/sbin/chown", args1);		// Mac
        }
#else
        {
            QStringList args1;
            args1 << "0:0" << pfn;
            execAsRoot("/bin/chown", args1);		// ubuntu
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

void OsSpecific::startPing(QProcess & pr, const QString & adr)
{
    pr.start(pingCommand(), formatArguments(adr));
}

int OsSpecific::extractPing(QProcess & pr)
{
    int ping = -1;
    QByteArray ba = pr.readAllStandardOutput();
    QString s(ba);
    QStringList out = s.split("\n", QString::SkipEmptyParts);
    if (!out.isEmpty()) {
        const QString & sp = out.at(out.size() - 1).trimmed();	// last line
#ifndef Q_OS_WIN
        if (sp.indexOf("min/avg/max") > -1) {
            int e = sp.indexOf('=');
            int slash = sp.indexOf('/', e +1);
            int sl1 = sp.indexOf('/', slash +1);
            if (sl1 > -1) {
                QString sv = sp.mid(slash + 1, sl1 - slash - 1);
                bool ok;
                double d = sv.toDouble(&ok);
                if (ok)
                    ping = (int)d;
            }
        }
#else
        int a;
        if ((a = sp.indexOf("Average =")) > -1) {
            int e = sp.indexOf('=', a);
            if (e > -1) {
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

QStringList OsSpecific::formatArguments(const QString & adr)
{
    QStringList args;
    args
#ifndef Q_OS_WIN
            << "-c" << "1"		// one packet - Mac, Linux
#ifdef Q_OS_LINUX
            << "-w" << "1"		// 1s deadline - Linux
#endif
#ifdef Q_OS_DARWIN
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

const QString & OsSpecific::pingCommand()
{
#ifdef  Q_OS_DARWIN
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

const QString gs_icon = ":/icons/icon-tray.png";
const QString gs_icon_cross = ":/icons/icon-tray-cross.png";
const QString gs_icon_cycle = ":/icons/icon-tray-cycle.png";

const QString gs_icon_white = ":/icons/icon-tray-white.png";
const QString gs_icon_cross_white = ":/icons/icon-tray-cross-white.png";
const QString gs_icon_cycle_white = ":/icons/icon-tray-cycle-white.png";

const QString gs_icon_light = ":/icons/icon-tray-hover.png";
const QString gs_icon_cross_light = ":/icons/icon-tray-hover-cross.png";
const QString gs_icon_cycle_light = ":/icons/icon-tray-hover-cycle.png";

const QString gs_icon_color = ":/icons/icon-tray-color.png";
const QString gs_icon_cross_color = ":/icons/icon-tray-color-cross.png";
const QString gs_icon_cycle_color = ":/icons/icon-tray-color-cycle.png";

const QString OsSpecific::disconnectedIcon() const
{
#ifdef Q_OS_DARWIN
    return isDark() ? gs_icon_cross_white : gs_icon_cross;
#else
    return gs_icon_cross_color;
#endif
}

const QString OsSpecific::connectingIcon() const
{
#ifdef Q_OS_DARWIN
    return isDark() ? gs_icon_cycle_white : gs_icon_cycle;
#else
    return gs_icon_cycle_color;
#endif
}

#ifdef Q_OS_DARWIN
bool OsSpecific::isDark() const
{
    QString result = runCommandFast("defaults read -g AppleInterfaceStyle");
    bool dark = result.contains("Dark");
    log::logt(QString("Current theme ") + result);
    return dark;
}
#endif

const QString OsSpecific::connectedIcon() const
{
#ifdef Q_OS_DARWIN
    return isDark() ? gs_icon_white : gs_icon;
#else
    return gs_icon_color;
#endif
}


const QString OsSpecific::disconnectedSelectedIcon() const
{
#ifdef Q_OS_DARWIN
    return gs_icon_cross_light;
#else
    return gs_icon_cross_color;
#endif
}

const QString OsSpecific::connectingSelectedIcon() const
{
#ifdef Q_OS_DARWIN
    return gs_icon_cycle_light;
#else
    return gs_icon_cycle_color;
#endif
}

const QString OsSpecific::connectedSelectedIcon() const
{
#ifdef Q_OS_DARWIN
    return gs_icon_light;
#else
    return gs_icon_color;
#endif
}

const char *OsSpecific::isOpenvpnRunningCommand()
{
#ifdef Q_OS_DARWIN
    return "ps -xa | grep open | grep execut | grep Safeju";
#else
    return "ps -xa | grep open | grep vpn | grep safej";
#endif
}

bool OsSpecific::isNetDown()
{
    return mNetDown;
}

void OsSpecific::setNetDown(bool b)
{
    mNetDown = b;
}

#ifdef Q_OS_WIN
static const wchar_t * gs_regpath = L"SYSTEM\\CurrentControlSet\\Services\\Tcpip6\\Parameters";
static const wchar_t * gs_regname = L"DisabledComponents";
struct RegRaii {
    RegRaii(HKEY & r) : _r(r) {}
    ~RegRaii()
    {
        RegCloseKey(_r);
    }
private:
    HKEY _r;
};
#endif

#ifdef Q_OS_OSX
static const QString gs_ns = "/usr/sbin/networksetup";
static const char * gs_wifi = "Wi-Fi";
static const char * gs_ether = "Ethernet";
#endif

void OsSpecific::setIPv6(bool enable)
{
    bool curr, ok;
    try {
        curr = IPv6();
        ok = true;
    } catch(...) {}

    bool doit = true;
    if (ok && (curr == enable))
        doit = false;
    if (doit) {
        log::logt("Changing IPv6 state to: " + QString(enable ? "enabled" : "disabled"));
#ifdef Q_OS_WIN
        HKEY hKey;
        LONG lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, gs_regpath, 0, KEY_READ | KEY_WRITE, &hKey);
        if (lRes != ERROR_SUCCESS) {
            throw std::runtime_error("Cannot open IPv6 reg key for writing");		// another error
        } else {
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
            execAsRoot(gs_ns, a);
        }
        {
            QStringList a;
            a << ac << gs_ether;
            execAsRoot(gs_ns, a);
        }
#else
        // throw std::logic_error("OsSpecific::SetIPv6() Not implemented for this OS");
#endif
#endif	// Q_OS_WIN
    }
}

#ifdef Q_OS_OSX
bool adapterHasIpv6(const char * adapter)
{
    bool has = true;
    QStringList a;
    a << "-getinfo" << adapter;
    QProcess pr;
    pr.start(gs_ns, a);
    if (pr.waitForFinished(1000)) {
        QString s = pr.readAllStandardOutput();
        if (s.indexOf("IPv6:") > -1) {
            has = true;
            if (s.indexOf("IPv6: Off") > -1)
                has = false;
        } else {
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
    if (lRes != ERROR_FILE_NOT_FOUND) {
        if (lRes == ERROR_SUCCESS) {
            RegRaii ra(hKey);
            DWORD val;
            DWORD sz = sizeof(val);
            lRes = ::RegQueryValueExW(hKey, gs_regname, 0, NULL, reinterpret_cast<LPBYTE>(&val), &sz);
            if (lRes != ERROR_FILE_NOT_FOUND) {
                if (lRes == ERROR_SUCCESS) {
                    if ( (val & 0x1) == 0x1)	// 0x01 to disable IPv6 on all tunnel interfaces https://support.microsoft.com/en-us/kb/929852
                        on = false;
                } else {
                    throw std::runtime_error("Cannot read IPv6 reg key value");		// another error
                }
            }
        } else {
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
    on =	adapterHasIpv6(gs_wifi);
    if (!on)
        on =	adapterHasIpv6(gs_ether);
#else
    //throw std::logic_error("OsSpecific::IPv6() Not implemented for this OS");
#endif
#endif	// Q_OS_WIN
    return on;
}

#ifdef Q_OS_WIN
void OsSpecific::enableTap()
{
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
    if (NO_ERROR != r) {
        log::logt("Cannot enumerate adapters, error code: " + QString::number(r));
    } else {
        for (; p != NULL; p = p->Next) {
            QString s = QString::fromWCharArray(p->Description);
            if (s.indexOf("TAP-Windows") > -1) {
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
}
#endif	// Q_OS_WIN

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
    if (lastn < 0) {
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

void OsSpecific::setStartup(bool b)
{
#ifdef Q_OS_WIN
    {
        QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
        static const char * keyname = "proxy_sh";
        if (!b)
        {
            settings.remove(keyname);
        } else
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
            if (d.mkpath(dir)) {
                QFile f(pfn);
                if (f.exists()) {
                    f.open(QIODevice::ReadWrite);
                    delete_startup(f);
                } else {
                    f.open(QIODevice::Append);
                }
                f.write(gs_desktop.toLatin1());
                f.flush();
                f.close();
            }
        } else
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
    if (pa.exists()) {
        if (!pa.remove())
            log::logt("Cannot delete startup file '"+ pfn + "'");
    }
    if (b) {
        QDir d(dir);
        if (!d.exists()) {
            d.mkpath(dir);
        }
        if (!pa.open(QIODevice::WriteOnly)) {
            log::logt("Cannot open startup file '"+ pfn + "' for writing");
        } else {

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

void OsSpecific::netDown()
{
    try {
#ifndef Q_OS_WIN
        setRights();
        log::logt("NetDown()");
        QString ss = runCommandFast(PathHelper::Instance()->netDownFilename());
        mNetDown = true;
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
        if (NO_ERROR != r) {
            log::logt("Cannot enumerate adapters, error code: " + QString::number(r));
        } else {
            for (int k = 1; p != NULL && k < 5; p = p->Next, ++k) {	// adpters in order - so try first 4
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
                    else {
                        log::logt("ShellExecute OK");
                        mNetDown = true;
                    }
                }
            }
        }
#endif	// Q_OS_WIN
    } catch(std::exception & ex) {
        log::logt(ex.what());
    }
}

QString OsSpecific::runCommandFast(const QString & cmd, uint16_t ms /* = 500 */) const
{
    return runCommandFast(cmd.toStdString().c_str(), ms);
}

QString OsSpecific::runCommandFast(const char * cmd, uint16_t ms /* = 500 */) const
{
    std::auto_ptr<QProcess> pr(new QProcess());
    pr->start(cmd);
    if (!pr->waitForFinished(ms)) {
        // or if this QProcess is already finished)
        if (pr->exitStatus() != QProcess::NormalExit) {
            QString s1(pr->readAllStandardError());
            log::logt("runCommandFast(): Error: not NormalExit " + s1);
        }
        if (QProcess::NotRunning != pr->state()) {
            pr->terminate();
            pr->kill();
        }

    }
    QString s0(pr->readAllStandardOutput());
    pr.release()->deleteLater();
    return s0;
}

bool OsSpecific::hasInsecureWifi()
{
    bool has = false;

#ifdef Q_OS_OSX
    std::vector<std::string> wifi_devices;
    QString s0 = runCommandFast("/usr/sbin/networksetup -listallhardwareports");
    QStringList sl0 = s0.split(QString("Hardware Port:"), QString::SkipEmptyParts);
    for (int k = 0; k < sl0.length(); ++k) {
        const QString & rs = sl0.at(k);
        if (rs.contains("Wi-Fi", Qt::CaseInsensitive) || rs.contains("Airport", Qt::CaseInsensitive)) {	// since 10.7 changed to Wi-Fi
            static const QString s_device = "Device: ";
            int p = rs.indexOf(s_device, 0, Qt::CaseInsensitive);
            if (p > -1) {
                int p1 = rs.indexOf('\n', p + s_device.length(), Qt::CaseInsensitive);
                if (p1 > -1) {
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
    for (size_t k = 0; !on && k < wifi_devices.size(); ++k) {
        std::string s = "/usr/sbin/networksetup -getairportpower " + wifi_devices.at(k);
        QString s1 = runCommandFast(s.c_str());
        if (s1.contains("on", Qt::CaseInsensitive))
            on = true;
    }

    if (on) {
        QString s1 = runCommandFast("/System/Library/PrivateFrameworks/Apple80211.framework/Versions/Current/Resources/airport -I");
        int p1 = s1.indexOf("link auth:", 0, Qt::CaseInsensitive);
        if (p1 > -1) {
            int p2 = s1.indexOf('\n', p1 + 1, Qt::CaseInsensitive);
            QString s2 = s1.mid(p1, p2 - p1 + 1);
            if (!s2.contains("wpa2", Qt::CaseInsensitive)) {
                has = true;
                log::logt("Detected unsecure Wi-Fi with " + s2.trimmed());
            }
        }
    }
#endif	// Q_OS_OSX

    return has;
}

void OsSpecific::fixDnsLeak()
{
#ifdef Q_OS_WIN
    QSysInfo::WinVersion v = QSysInfo::windowsVersion();
    if (v >= QSysInfo::WV_WINDOWS8) {
        QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows NT\\DNSClient", QSettings::NativeFormat);
        QVariant val = 1;
        settings.setValue("DisableSmartNameResolution", val);
    }
#endif
}

void OsSpecific::stopObfsproxy()
{
    if (obfsproxyRunning()) {
        if (mObfsproxy.get()) {
            mObfsproxy->terminate();
            mObfsproxy->kill();
            mObfsproxy.release()->deleteLater();
            while (obfsproxyRunning()) { // Make sure it stopped so it will start
                QThread::sleep(100);
            }
        }
    }
}

void OsSpecific::runObfsproxy(const QString &srv,
                              const QString &port,
                              const QString &obfstype,
                              const QString & local_port)
{
    log::logt("runObfsproxy called with server " + srv + " and type " + obfstype);
    if (!obfsproxyInstalled()) {
#ifndef Q_OS_WIN
        installObfsproxy();
#endif
    }

    log::logt("SRV = " + srv);

    if (!obfsproxyRunning()) {
        QStringList args;
        args << PathHelper::Instance()->obfsproxyFilename();

#ifndef Q_OS_WIN
        args << "--log-file " + PathHelper::Instance()->obfsproxyLogFilename();
        args << "--data-dir /tmp";
#endif
        args << obfstype;
        if (obfstype.compare("scramblesuit", Qt::CaseInsensitive) == 0) {
            args << "--password JNI3L3K2VZM3UY37WEA2JQ442V5YVZZS";
        }
        args << "--dest " + srv + ":" + port;
    //	"185.47.202.158:888 "
        args << "socks 127.0.0.1:" + local_port;
    //	"1050"
        const QString cmd = args.join(' ');

        mObfsproxy.reset(new QProcess());
        connect(mObfsproxy.get(), SIGNAL(finished(int,QProcess::ExitStatus)),
                this, SLOT(obfsFinished(int,QProcess::ExitStatus)));
        mObfsproxy->setStandardErrorFile(PathHelper::Instance()->obfsproxyLogFilename());
        mObfsproxy->setStandardOutputFile(PathHelper::Instance()->obfsproxyLogFilename());
        log::logt("Executing obfsproxy with command " + cmd);
        mObfsproxy->start(cmd);
        QThread::msleep(100);
        log::logt(QString("if (!IsObfsRunning()) ") + cmd);
    }
    log::logt("runObfsproxy() done");
}

void OsSpecific::installObfsproxy()
{
    log::logt("InstallObfs() in");
#ifdef Q_OS_LINUX

#ifndef Q_OS_REDHAT
    // debian/ubuntu
    execAsRoot("apt-get", QStringList() << "update");
    execAsRoot("apt-get",  QStringList() << "-y" << "install" << "python-dev");
    execAsRoot("wget",  QStringList() <<  "https://bootstrap.pypa.io/get-pip.py" << "-O" << "/tmp/get-pip.py");
    execAsRoot("python",  QStringList() <<  "/tmp/get-pip.py");
#else
    // CentOS
    execAsRoot("yum",  QStringList() <<  "-y" << "install" << "python-devel");
    execAsRoot("/usr/bin/curl",  QStringList() <<  "https://bootstrap.pypa.io/get-pip.py" << "-o" << "/tmp/get-pip.py");
    execAsRoot("python",  QStringList() <<  "/tmp/get-pip.py");
#endif
    execAsRoot("pip",  QStringList() << "install" << "obfsproxy");
#else
#ifdef Q_OS_DARWIN
    log::logt("Show notification");
    int ii = WndManager::Instance()->Confirmation("Installing OBFS proxy");
    execAsRoot("/usr/bin/easy_install", QStringList() << "pip");
    execAsRoot("/usr/local/bin/pip", QStringList() << "install" << "virtualenv");
    execAsRoot(PathHelper::Instance()->installObfsproxyFilename(), QStringList());

    while (!obfsproxyInstalled()) {
        QThread::msleep(400);
    }
#endif	// Q_OS_DARWIN
#endif	// Q_OS_LINUX
    log::logt("InstallObfs() out");
}

void OsSpecific::obfsFinished(int exitCode, QProcess::ExitStatus status)
{
    log::logt("obfsFinished with code " + QString::number(exitCode) +
              " and status " + QString::number(status));
    delete mObfsproxy.release();
    emit obfsproxyFinished();
}

bool OsSpecific::obfsproxyInstalled()
{
    bool b = true;
#ifdef Q_OS_DARWIN
    QFile f(PathHelper::Instance()->resourcesPath() + "/env/bin/obfsproxy");
    b = f.exists();
#else
#ifndef Q_OS_WIN
    QString s = runCommandFast("which obfsproxy", 1000);
    b = !s.isEmpty();
    log::logt("WHICH OBFS " + s + b);
#endif
#endif
    log::logt(QString("IsObfsInstalled ") + (b ? "TRUE" : "FALSE"));
    return b;
}

bool OsSpecific::obfsproxyRunning()
{
    bool b = false;
#ifdef Q_OS_WIN
    if (mObfsproxy.get() != NULL) {
        b = true;		// TODO: -1 actual check
    }
#else
    {
        QString s1 = runCommandFast("ps ax");
        b = s1.contains("/obfsproxy");
        QString result = b ? "true" : "false";
        log::logt("IsObfsRunning result: " + result);
    }
#endif
    return b;
}


