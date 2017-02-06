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

#ifndef OSSPECIFIC_H
#define OSSPECIFIC_H

#include <memory>
#include <stdint.h>

#include <QString>
#include <QProcess>

class OsSpecific: public QObject
{
    Q_OBJECT
public:
    static bool exists();
    static OsSpecific *instance();
    static void cleanup();
    ~OsSpecific();


    void execAsRoot(const QString & cmd, const QStringList & argv);	// throw std::exception on error
#ifndef Q_OS_WIN
    void setOwnerRoot(const QString & pfn);			// throw std::exception on error
#endif

    // check  and set rights for all the needed files; {chmod, chown, chgrp};
    // the only place requires local elevated rights
    // throw std::exception on error
    void setRights();

    void setStartup(bool b);		// Auto-launch app on startup

    void startPing(QProcess & pr, const QString & adr);		// pr must have already connected finished() signal
    int extractPing(QProcess & pr);		// exract ping value from pr's stdout; -1 on error / unavailable

    void setIPv6(bool enable);		// throw std::exception on error
    bool IPv6();	// test OS and return enabled	// throw std::exception on error
    bool hasInsecureWifi();
    void fixDnsLeak();
    void netDown();

    void enableTap();	// for Windows enumerate interfaces and for all TAP force call Enable

    const QString disconnectedIcon() const;
    const QString connectingIcon() const;
    const QString connectedIcon() const;

    const QString disconnectedSelectedIcon() const;
    const QString connectingSelectedIcon() const;
    const QString connectedSelectedIcon() const;

    const char * isOpenvpnRunningCommand();

    bool isNetDown();
    void setNetDown(bool b);

    QString runCommandFast(const char * cmd, uint16_t ms = 500);			// returns stdout
    QString runCommandFast(const QString & cmd, uint16_t ms = 500);

    void runObfsproxy(const QString &srv,
                      const QString &port,
                      const QString &obfstype,
                      const QString &local_port);
    bool obfsproxyRunning();
    void stopObfsproxy();
    bool obfsproxyInstalled();

    void installObfsproxy();

#ifdef Q_OS_MAC
    bool isDark() const;
#endif
private slots:
    void obfsFinished(int exitCode, QProcess::ExitStatus status);
private:
    OsSpecific();
    static std::auto_ptr<OsSpecific> mInstance;
    std::auto_ptr<QProcess> mObfsproxy;

    void setChmod(const char * sflags, const QString & pfn);		// flags in form 04555 - will by parsed in both 8- and 16-based
    void setChown(const QString & pfn);

    bool isOwnerRoot(const QString & pfn);

    void releaseRights();

    const QString & pingCommand();			// both for StartPing()
    QStringList formatArguments(const QString & adr);
    bool mNetDown;

};

#endif // OSSPECIFIC_H
