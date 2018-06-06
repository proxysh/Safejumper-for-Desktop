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

#include <QCoreApplication>
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
#include <errno.h>

#include "common.h"
#include "log.h"

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
{}

OsSpecific::~OsSpecific()
{
}

QString OsSpecific::runCommandFast(const QString & cmd, uint16_t ms /* = 500 */) const
{
    return runCommandFast(cmd.toStdString().c_str(), ms);
}

QString OsSpecific::runCommandFast(const QString &cmd, const QStringList &arguments)
{
    std::auto_ptr<QProcess> pr(new QProcess());
    pr->start(cmd, arguments);
    if (!pr->waitForFinished()) {
        // or if this QProcess is already finished)
        if (pr->exitStatus() != QProcess::NormalExit) {
            QString s1(pr->readAllStandardError());
            Log::logt("runCommandFast(): Error: not NormalExit " + s1);
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

QString OsSpecific::runCommandFast(const char * cmd, uint16_t ms /* = 500 */) const
{
    std::auto_ptr<QProcess> pr(new QProcess());
    pr->start(cmd);
    if (!pr->waitForFinished(ms)) {
        // or if this QProcess is already finished)
        if (pr->exitStatus() != QProcess::NormalExit) {
            QString s1(pr->readAllStandardError());
            Log::logt("runCommandFast(): Error: not NormalExit " + s1);
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
                Log::logt("Detected unsecure Wi-Fi with " + s2.trimmed());
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
