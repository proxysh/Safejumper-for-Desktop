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

#ifndef OPENVPN_MANAGER_H
#define OPENVPN_MANAGER_H

#include <memory>
#include <QDateTime>
#include <QNetworkReply>
#include <QObject>
#include <QProcess>
#include <QTcpSocket>
#include <QFileSystemWatcher>
#include <QTemporaryFile>

#include "common.h"

#define kTryNextPortSeconds 60
#define G_Delay_PortIteration 80

// only 5 seconds for each TCP connection
#define G_Delay_OneCheck 30
#define G_Max_Reconnect 3

class QTimer;

class OpenvpnManager: public QObject
{
    Q_OBJECT
public:
    ~OpenvpnManager();
    static OpenvpnManager * instance();
    static void cleanup();
    static bool exists();

    // Set testing property, if true logs are deleted between runs
    void setTesting(bool testing);

    // Set whether we should try to reconnect if connection fails
    void setReconnect(bool reconnect);

    // Set vpn credentials
    void setVPNCredentials(const QString &username, const QString &password);

    void start(int encryption,
               const QString &server,
               const QString &port,
               const QString &localPort,
               const QString &tcpOrUdp,
               bool disableIPV6,
               bool fixDNS,
               const QString &dns1,
               const QString &dns2);
    void cancel(const QString & msg);
    void stop();			// normal stop of executed via this process
    void killRunningOpenvpn();		// kill process executed previously: connect to it first, if fails: kill -9

    bool openvpnRunning();

    vpnState state();

    void startPortLoop(bool port);		// true - cycle ports; false - cycle nodes

    void netDown(bool down = true);

signals:
    void gotNewIp(const QString &ip);

    void stateChanged(vpnState state);

    void stateWordChanged(OpenVPNStateWord word);

    void sendError(const QString &errorMessage);

    void timedOut();

    // Tell gui to block since we are disconnected
    void blockOnDisconnect();

    void switchingPort(const QString &port);

private slots:
    void socketConnected();
    void socketError(QAbstractSocket::SocketError error);
    void socketReadyRead();
    void reconnectTimeout();
    void openvpnLogfileChanged(const QString & pfn);		// OpenVpn log file
    void checkState();		// timer calls it
    void obfsFinished(int exitCode, QProcess::ExitStatus status);

    void processError(QProcess::ProcessError error);
    void processStarted();
    void processStateChanged(QProcess::ProcessState st);
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void logStderr();
    void logStdout();

    void getOvpnError(QNetworkReply::NetworkError error);
    void getOvpnFinished();

private:
    OpenvpnManager();
    void startConnecting(); // Start connecting with the current settings
    void connectToOpenvpnSocket();	// attach to OpenVPN management socket
    void disconnectFromOpenvpnSocket();
    void removeProcess();
    void setupFileWatcher();
    void parseOpenvpnLogLine(const QString & s);
    void parseNewIp(const QString & s);
    void gotConnected(const QString & s);
    void gotTunErr(const QString &);
    void showErrorMessageCleanup(QString msg);
    void launchObfsproxy(); // Executes obfsproxy with current settings
    void runObfsproxy(const QString &srv,
                      const QString &port,
                      const QString &obfstype,
                      const QString &local_port);
    bool obfsproxyRunning();
    void stopObfsproxy();

    bool writeConfigFile(); // Write openvpn options to ovpn file
    QStringList getOpenvpnArgs(); // Arguments to pass to openvpn

    void parseSocketLine(QString s);
    void parseSocketQueryWord(const QString & word, const QString & s);
    void parseSocketPlainWord(const QString & word, const QString & s);
    void parseSocketStateWord(const QString & word, const QString & s);

    void startTimer();
    void startReconnectTimer();

    void setState(vpnState st);

    // check  and set rights for all the needed files; {chmod, chown, chgrp};
    // the only place requires local elevated rights
    // throw std::exception on error
    void setRights();
    void setChmod(const char * sflags, const QString & pfn);		// flags in form 04555 - will by parsed in both 8- and 16-based
    void setChown(const QString & pfn);

    void setIPv6(bool enable);		// throw std::exception on error
    bool IPv6();	// test OS and return enabled	// throw std::exception on error

#ifdef Q_OS_WIN
    void enableTap();	// for Windows enumerate interfaces and for all TAP force call Enable
#endif

    void getOvpn();
    void continueConnecting();

    static std::auto_ptr<OpenvpnManager> mInstance;

    std::auto_ptr<QFileSystemWatcher> mFileSystemWatcher;	// OpenVpn log file
    std::auto_ptr<QTemporaryFile> mParametersTempFile;
    std::auto_ptr<QProcess> mProcess;
    std::auto_ptr<QProcess> mObfsproxy;
    vpnState mState;
    QTimer *mStateTimer;
    int mPID;		// for running process (run gui after crash)
    std::auto_ptr<QTcpSocket> mSocket;

    qint64 mLogFilePosition;
    bool mProcessing;
    bool mTunError;
    bool mError;

    QString mPreviousStateWord;

    int mReconnectAttempt;
    QDateTime mStartDateTime;

    bool mTesting;

    QString mVPNUsername;
    QString mVPNPassword;

    QString mHostname;
    QString mPort;
    QString mLocalPort;
    QString mTcpOrUdp;
    int mEncryption;
    bool mReconnect;
    bool mFixDNS;
    bool mDisableIPv6;
    QString mDNS1;
    QString mDNS2;

    bool mNetDown;

    QByteArray mOvpnContent;
};

#endif // OPENVPNMANAGER_H
