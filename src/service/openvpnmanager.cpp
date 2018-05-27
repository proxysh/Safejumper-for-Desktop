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

#include "openvpnmanager.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QThread>
#include <QTimer>
#include <stdexcept>

#include "servicelog.h"
#include "osspecific.h"
#include "servicepathhelper.h"

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <windows.h>
#include <Iphlpapi.h>
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#endif

#ifndef Q_OS_WIN
#include <unistd.h>
#include <sys/stat.h>		// chmod
#endif

const QString kLocalAddress = "127.0.0.1";

OpenvpnManager::OpenvpnManager()
    : mState(vpnStateDisconnected),
      mStateTimer(NULL),
      mPID(0),
      mSocket(0),
      mTesting(false),
      mNetDown(false)
{
}

OpenvpnManager::~OpenvpnManager()
{
    stop();
}

void OpenvpnManager::cleanup()
{
    if (mInstance.get() != NULL)
        delete mInstance.release();
}

bool OpenvpnManager::exists()
{
    return (mInstance.get() != NULL);
}

void OpenvpnManager::setTesting(bool testing)
{
    mTesting = testing;
}

void OpenvpnManager::setReconnect(bool reconnect)
{
    mReconnect = reconnect;
}

void OpenvpnManager::setVPNCredentials(const QString &username, const QString &password)
{
    mVPNUsername = username;
    mVPNPassword = password;
}

vpnState OpenvpnManager::state()
{
    return mState;
}

std::auto_ptr<OpenvpnManager> OpenvpnManager::mInstance;
OpenvpnManager * OpenvpnManager::instance()
{
    if (!mInstance.get())
        mInstance.reset(new OpenvpnManager());
    return mInstance.get();
}

void OpenvpnManager::start(int encryption,
                           const QString &server,
                           const QString &port,
                           const QString &localPort,
                           const QString &tcpOrUdp,
                           bool disableIPV6,
                           bool fixDNS,
                           const QString &dns1,
                           const QString &dns2)
{
    mReconnectAttempt = 0;

    stop();
    // TODO: -1 cleanup

    mEncryption = encryption;
    mHostname = server;
    mPort = port;
    mLocalPort = localPort;
    mTcpOrUdp = tcpOrUdp;
    mFixDNS = fixDNS;
    mDisableIPv6 = disableIPV6;
    mDNS1 = dns1;
    mDNS2 = dns2;

    startConnecting();
}

void OpenvpnManager::startConnecting()
{
    if (mTesting) {
        // If testing, nuke the log files between launches
        QFile::remove(ServicePathHelper::Instance()->openvpnLogFilename());
        QFile::remove(ServicePathHelper::Instance()->safejumperLogFilename());
    }
    setState(vpnStateConnecting);
    launchObfsproxy(); // Only launches if we are using obfs protocols
    try {
        setIPv6(!mDisableIPv6);
#ifdef Q_OS_WIN
        enableTap(); // TODO check win10 tap
#endif
    } catch(std::exception & ex) {
        Log::serviceLog(ex.what());
        if (mDisableIPv6) {
            emit sendError(QString("Cannot disable IPv6") + ex.what());
            return;
        }
    }

    mTunError = false;
    mError = false;
    mProcessing = false;

#ifndef NO_PARAMFILE
    if (!writeConfigFile()) {
        return; // Error, so return
    }
#endif  // NO_PARAMFILE

    QStringList args = getOpenvpnArgs();
    if (mFixDNS ||
            !mDNS1.isEmpty() ||
            !mDNS2.isEmpty())
        OsSpecific::instance()->fixDnsLeak();

    QString prog = ServicePathHelper::Instance()->openvpnFilename();
    Log::serviceLog("Prog is: " + prog);
    QString params = args.join(' ');
    Log::serviceLog("Args are:" + params);

    bool ok = false;
    try {
#ifndef Q_OS_WIN
        mParametersTempFile.reset(new QTemporaryFile());
        if (!mParametersTempFile->open())
            throw std::runtime_error("Cannot create tmp file.");

        setRights();        // lean inside, throw on error

        mParametersTempFile->write(params.toLatin1());
        mParametersTempFile->flush();

        QStringList arg3;
        arg3 << mParametersTempFile->fileName();
        mParametersTempFile->close();

//#ifdef Q_OS_DARWIN
//          Log::serviceLog("######  touch launcher ####");
//          OsSpecific::Instance()->RunFastCmd("touch -a " + ServicePathHelper::Instance()->LauncherPfn());
//#endif
        Log::serviceLog("######  before exec ####");

#ifdef Q_OS_LINUX
        QProcess::execute(ServicePathHelper::Instance()->launchopenvpnFilename(), arg3);
#else
//#ifdef Q_OS_DARWIN
//            QProcess::execute(prog, args);     // force password dialog; without launcher
//#else
        int r3 = QProcess::execute(ServicePathHelper::Instance()->launchopenvpnFilename(), arg3);    // 30ms block internally
        Log::serviceLog("Launched " + ServicePathHelper::Instance()->launchopenvpnFilename() + " with argument " + arg3.at(0));
        Log::serviceLog("QProcess::execute() returns " + QString::number(r3));
        Log::serviceLog("###############");
        if (r3 != 0) {
            std::string ts;
            switch (r3) {
            case -2:        // cannot be started
            case -1: {      // the process crashes
                ts = ("OpenVPN couldn't be reached (" + QString::number(r3) + "). Please reboot and/or re-install Safejumper.").toStdString();
                break;
            }
            default:
                ts = ("Cannot run OpenVPN. Error code is: " + QString::number(r3)).toStdString();
                break;
            }
            throw std::runtime_error(ts);
        }
//#endif
#endif

        // Wait 1 seconds to let openvpn open the socket we connect to
        sleep(1);
        Log::serviceLog("before attaching to OpenVPN");
        connectToOpenvpnSocket();   // TODO: -1 wait for slow starting cases
        setupFileWatcher();
        Log::serviceLog("after attaching to OpenVPN");
#else
        mProcess.reset(new QProcess());
        connect(mProcess.get(), SIGNAL(error(QProcess::ProcessError)),
                this, SLOT(processError(QProcess::ProcessError)));
        connect(mProcess.get(), SIGNAL(started()),
                this, SLOT(processStarted()));
        connect(mProcess.get(), SIGNAL(stateChanged(QProcess::ProcessState)),
                this, SLOT(processStateChanged(QProcess::ProcessState)));
        connect(mProcess.get(), SIGNAL(finished(int, QProcess::ExitStatus)),
                this, SLOT(processFinished(int, QProcess::ExitStatus)));
        connect(mProcess.get(), SIGNAL(readyReadStandardError()),
                this, SLOT(logStderr()));
        connect(mProcess.get(), SIGNAL(readyReadStandardOutput()),
                this, SLOT(logStdout()));
        mProcess->start(prog, args);
        mProcess->waitForStarted(2000);
        Log::serviceLog("Process ID is: " + QString::number(mProcess->processId()));
#endif
        ok = true;
    } catch(std::exception & ex) {
        Log::serviceLog(ex.what());
        emit sendError(ex.what());
    }

    if (mParametersTempFile.get())
        delete mParametersTempFile.release();

    if (ok) {
        Log::serviceLog("openvpn launched, starting timer");
        mStartDateTime = QDateTime::currentDateTimeUtc();
        startTimer();
    } else
        setState(vpnStateDisconnected);

    Log::serviceLog("launchopenvpn done");
#undef NO_PARAMFILE
}

void OpenvpnManager::launchObfsproxy()
{
    bool obfs = (mEncryption == ENCRYPTION_TOR_OBFS2
                 || mEncryption == ENCRYPTION_TOR_OBFS3
                 || mEncryption == ENCRYPTION_TOR_SCRAMBLESUIT
                );
    if (mHostname.isEmpty() || mPort.isEmpty()) {
        QString message = "Server or port is empty, select a location";
        Log::serviceLog(message);
        emit sendError(message);
        return;
    }
    if (obfs) {
        QString obfstype;
        if (mEncryption == ENCRYPTION_TOR_OBFS2)
            obfstype = "obfs2";
        else if (mEncryption == ENCRYPTION_TOR_OBFS3)
            obfstype = "obfs3";
        else
            obfstype = "scramblesuit";
        runObfsproxy(mHostname, mPort, obfstype, "1050");
        if (!obfsproxyRunning()) {
            Log::serviceLog("Cannot run Obfsproxy");
            emit sendError("Cannot run Obfsproxy");
            return;
        }
    }
}

void OpenvpnManager::processError(QProcess::ProcessError error)
{
    Log::serviceLog("OpenvpnManager::processError(): error = " + QString::number(error));
    emit stateChanged(vpnStateDisconnected);
}

void OpenvpnManager::processStarted()
{
    Log::serviceLog("OpenvpnManager::ConnectStarted()");
}

bool OpenvpnManager::writeConfigFile()
{
    bool obfs = (mEncryption == ENCRYPTION_TOR_OBFS2
                 || mEncryption == ENCRYPTION_TOR_OBFS3
                 || mEncryption == ENCRYPTION_TOR_SCRAMBLESUIT
                );
    QFile ff(ServicePathHelper::Instance()->openvpnConfigFilename());
    if (!ff.open(QIODevice::WriteOnly)) {
        QString se = "Cannot write config file '" + ServicePathHelper::Instance()->openvpnConfigFilename() + "'";
        Log::serviceLog(se);
        emit sendError(se);
        return false;
    }
    ff.write("client\n");
    ff.write("dev tun\n");
    ff.write("setenv FORWARD_COMPATIBLE 1\n");
    ff.write("proto ");
    ff.write(mTcpOrUdp.toLatin1());
    ff.write("\n");       // "tcp"/"udp"
//ff.write("proto udp\n");
//ff.write("proto tcp\n");

    if (mHostname.isEmpty() || mPort.isEmpty()) {
        QString message = "Server or port is empty, select a location";
        Log::serviceLog(message);
        emit sendError(message);
        return false;
    }
    ff.write("remote ");
    ff.write(mHostname.toLatin1());
    ff.write(" ");
    ff.write(mPort.toLatin1());
    ff.write("\n");

//ff.write("remote 176.67.168.144 465\n");  // france 7
//ff.write("remote 217.18.246.133 465\n");      // levski.proxy.sh

//ff.write("remote 176.9.137.187 465\n");   // germany 9 :ecc

//ff.write("remote 176.67.168.144 995\n");  // ecc+xor

    ff.write("cipher AES-256-CBC\n");
    ff.write("auth SHA512\n");
    ff.write("auth-user-pass\n");
    ff.write("remote-cert-tls server\n");
    ff.write("resolv-retry infinite\n");
    ff.write("nobind\n");
    ff.write("comp-lzo\n");
    ff.write("verb 3\n");
    ff.write("reneg-sec 0\n");
    ff.write("route-method exe\n");
    ff.write("route-delay 2 0\n");

    if (mEncryption == ENCRYPTION_ECC || mEncryption == ENCRYPTION_ECCXOR) {
//          ff.write("tls-cipher ECDHE-ECDSA-AES256-GCM-SHA384\n");
        ff.write("tls-cipher TLS-ECDHE-ECDSA-WITH-AES-256-GCM-SHA384\n");

        //ff.write("tls-cipher ECDH\n");
        //ff.write("tls-cipher !ECDH\n");

        ff.write("ecdh-curve secp384r1\n");

        if (mEncryption == ENCRYPTION_ECCXOR)
            ff.write("scramble obfuscate 0054D65beN6r2kd\n");

        // TODO: -1 download cert from https://proxy.sh/proxysh-ecc.crt
        ff.write(
            "<ca>\n"
            "-----BEGIN CERTIFICATE-----\n"
            "MIIB3DCCAWKgAwIBAgIJAMyliDCXM4kcMAoGCCqGSM49BAMCMBMxETAPBgNVBAMT\n"
            "CHByb3h5LnNoMB4XDTE0MTExMzExNTk1NVoXDTI0MTExMDExNTk1NVowEzERMA8G\n"
            "A1UEAxMIcHJveHkuc2gwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAATwczmfgxUfobt/\n"
            "7S+A2P1tYNOYATTpxcIEAtUVCgywp1Fd6tKAttCqvpHz8PDOb4NYS6JONivO5yaT\n"
            "jfDiTrWRGZeYf2JsNs6byv/A9qxvDCcJ49EotonMJYX4+TQq75ejgYEwfzAdBgNV\n"
            "HQ4EFgQU6miAiqVUQAYeUP4LnZfKNdfQjUkwQwYDVR0jBDwwOoAU6miAiqVUQAYe\n"
            "UP4LnZfKNdfQjUmhF6QVMBMxETAPBgNVBAMTCHByb3h5LnNoggkAzKWIMJcziRww\n"
            "DAYDVR0TBAUwAwEB/zALBgNVHQ8EBAMCAQYwCgYIKoZIzj0EAwIDaAAwZQIwd5vR\n"
            "8fTrEdXLKZjiXeCjH/vxnnbelGcgpFz/r0cdr8YISa20w2zfGVB1+8XRhaYHAjEA\n"
            "yZeceiNW01Uj7DnjgWdLJWxcuduP1eTojzcQTGcFRPGd45w6pM1oGvLBhCD+QDzw\n"
            "-----END CERTIFICATE-----\n"
            "</ca>\n"
        );
    }

    if (mFixDNS) {
        qDebug() << "Fixdns is enabled, so setting dns servers";
#ifdef Q_OS_WIN
        if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS10)
            ff.write("block-outside-dns\n");
#endif
    }

    if (!mDNS1.isEmpty()) {
        ff.write(QString("dhcp-option DNS %1\n").arg(mDNS1).toUtf8());
    }
    if (!mDNS2.isEmpty()) {
        ff.write(QString("dhcp-option DNS %1\n").arg(mDNS2).toUtf8());
    }

    if (obfs) {
// TODO: -0 OS
        ff.write("socks-proxy 127.0.0.1 1050\n");
        ff.write("route ");
        ff.write(mHostname.toLatin1());
        ff.write(" 255.255.255.255 net_gateway\n");
    }

    ff.flush();
    ff.close();
    return true;
}

QStringList OpenvpnManager::getOpenvpnArgs()
{
    QStringList args;
//          << "--auth-nocache"
#ifndef NO_PARAMFILE
    args << "--config" << ServicePathHelper::Instance()->openvpnConfigFilename(); // /tmp/proxysh.ovpn
#endif
#ifdef NO_PARAMFILE
    args << "--client"
#endif

#ifndef Q_OS_WIN
         args << "--daemon"; // does not work at windows
#endif

#ifdef NO_PARAMFILE
    args << "--dev tun0"
         << "--proto" << Setting::Instance()->Protocol()
         << "--remote-random"
         << "--remote" << mHostname << mPort

         << "--cipher" << "AES-256-CBC"
         << "--auth" << "SHA512"
         << "--remote-cert-tls" << "server"

         << "--auth-user-pass"

         << "--resolv-retry" << "infinite"
         << "--nobind"
         << "--persist-key"
         << "--persist-tun"

         << "--verb" << "3"
         << "--comp-lzo"
         << "--route-delay" << "2"
         << "--allow-pull-fqdn";
#endif
    args << "--management" << kLocalAddress << mLocalPort
         << "--management-hold"
         << "--management-query-passwords";
    args << "--log" << ServicePathHelper::Instance()->openvpnLogFilename();           // /tmp/openvpn.log

//          << "--script-security" << "3" << "system"
//          << "--script-security" << "2" << "execve"       // https://openvpn.net/index.php/open-source/documentation/manuals/69-openvpn-21.html
    args << "--script-security" << "3";

#ifndef Q_OS_WIN            // TODO: -0 DNS on linux
    args << "--up" << ServicePathHelper::Instance()->upScriptFilename();     // /Applications/Safejumper.app/Contents/Resources/client.up.safejumper.sh
    args << "--down" << ServicePathHelper::Instance()->downScriptFilename(); // /Applications/Safejumper.app/Contents/Resources/client.down.safejumper.sh
#endif
    args << "--up-restart";

    // TODO: -1 download cert from proxy.sh
    if (mEncryption != ENCRYPTION_ECC && mEncryption != ENCRYPTION_ECCXOR)
        args << "--ca" << ServicePathHelper::Instance()->proxyshCaCertFilename();    // /tmp/proxysh.crt

    if (!mDNS1.isEmpty())
        args << "--dhcp-option" << "DNS" << mDNS1;
    if (!mDNS2.isEmpty())
        args << "--dhcp-option" << "DNS" << mDNS2;
    return args;
}

void OpenvpnManager::startTimer()
{
    if (mStateTimer != NULL)
        mStateTimer->stop();
    delete mStateTimer;
    mStateTimer = new QTimer(this);
    connect(mStateTimer, SIGNAL(timeout()), this, SLOT(checkState()));
    mStateTimer->start(1200);
}

void OpenvpnManager::checkState()
{
    Log::serviceLog("checkState called");
    if (openvpnRunning()) {
        if (mSocket.get() == NULL) {
            connectToOpenvpnSocket();
        } else if (mSocket->isValid() &&
                   mSocket->state() == QAbstractSocket::ConnectedState) {
            mSocket->write("state\n");
            mSocket->flush();
        } else {
            Log::serviceLog("Socket is not null, but not valid or connected");
        }
    } else if (state() == vpnStateConnected) {
        // handle crush
        if (mReconnect)
            startConnecting();
        else
            setState(vpnStateDisconnected);
    }

    if (state() == vpnStateConnecting) {
        uint d = mStartDateTime.secsTo(QDateTime::currentDateTimeUtc());
        if (mTesting) {
            if (d > kTryNextPortSeconds) {
                cancel(QString("Timeout at %1 seconds").arg(kTryNextPortSeconds));
            }
        } else if (d > kTryNextPortSeconds) {
            mStartDateTime = QDateTime::currentDateTimeUtc(); // Update start date time to now to prevent duplicate timeouts
            stop();
            emit timedOut();
        }

    }
    Log::serviceLog("checkState done");
}

void OpenvpnManager::openvpnLogfileChanged(const QString & pfn)
{
    if (mProcessing || mError)
        return;
    mProcessing = true;
    if (pfn == ServicePathHelper::Instance()->openvpnLogFilename()) {
        QFile f(pfn);
        QByteArray ba;
        if (f.open(QIODevice::ReadOnly)) {      // TODO: -2 ensure non-blocking
            if (mLogFilePosition > f.size())        // file was truncated
                mLogFilePosition = 0;
            if ((mLogFilePosition + 1) < f.size()) {
                f.seek(mLogFilePosition);
                ba = f.read(f.size() - mLogFilePosition);
                mLogFilePosition = f.size();
            }
            f.close();
        }
        if (!ba.isEmpty()) {
            QString s1(ba);
            QStringList sl = s1.split('\n', QString::SkipEmptyParts);
            for (int k = 0; k < sl.size(); ++k)
                parseOpenvpnLogLine(sl[k]);
        }
    }
    mProcessing = false;
}

void OpenvpnManager::parseOpenvpnLogLine(const QString & s)
{
    // TODO: -2 state machine
    if (s.contains("MANAGEMENT: CMD 'state'", Qt::CaseInsensitive))
        return;   // skip our commands

    Log::serviceLog("OpenVPNlogfile: " + s);

    // Check for used port
    if (s.contains("Socket bind failed on local address")) {
        QString port = mPort;
        // Change to next port
        mPort = QString::number(mPort.toInt() + 1);
        emit switchingPort(mPort);
        startConnecting(); // Try again
    }

    if (s.contains("TCPv4_CLIENT link remote:", Qt::CaseInsensitive)) {
        parseNewIp(s);
    } else if (s.contains("Initialization Sequence Completed:", Qt::CaseInsensitive)) {
        gotConnected(s);
    } else if (s.contains("Opening utun (connect(AF_SYS_CONTROL)): Operation not permitted", Qt::CaseInsensitive)) {
        gotTunErr(s);
    }
}

void OpenvpnManager::setState(vpnState st)
{
    if (st != mState) {
        mState = st;
        Log::serviceLog("Set state " + QString::number(st));
        switch (st) {
        case vpnStateConnected: {
            emit stateChanged(vpnStateConnected);
            break;
        }
        case vpnStateConnecting: {
            emit stateChanged(vpnStateConnecting);
            break;
        }
        case vpnStateDisconnected: {
            if (mStateTimer != NULL) {
                mStateTimer->stop();
                delete mStateTimer;
                mStateTimer = NULL;
            }
            emit stateChanged(vpnStateDisconnected);
            break;
        }
        default:
            break;
        }
    }
}

void OpenvpnManager::gotConnected(const QString & s)
{
    // extract IP
    //1432176303,CONNECTED,SUCCESS,10.14.0.6,91.219.237.159
    // 1460435651,CONNECTED,SUCCESS,10.200.1.6,85.236.153.236,465,192.168.58.170,35331
    int p = -1;
    for (int k = 0; k < 4; ++k)
        p = s.indexOf(',', p + 1);
    if (p > -1) {
        int p1 = s.indexOf(',', p + 1);
        QString ip = p1 > -1 ? s.mid(p + 1, p1 - p - 1) : s.mid(p + 1);
        if ((mEncryption != ENCRYPTION_TOR_OBFS2)
                && mEncryption != ENCRYPTION_TOR_OBFS3
                && mEncryption != ENCRYPTION_TOR_SCRAMBLESUIT
           ) // for proxy it shows 127.0.0.1
            emit gotNewIp(ip);
    }

    // Set state to connected after checking the new ip so we will have the new ip
    setState(vpnStateConnected);
}

void OpenvpnManager::gotTunErr(const QString & /* s */)
{
    if (!mTunError && !mError) {
        mTunError = true;
        mError = true;
        Log::serviceLog("Got TUN Allocation error, reconnecting");
        startConnecting();
    }
}

void OpenvpnManager::parseNewIp(const QString & s)
{
    // Tue May 1 03:50:58 2015 TCPv4_CLIENT link remote: [AF_INET]50.31.252.10:443
    int p0 = s.indexOf("TCPv4_CLIENT link remote");
    p0 = s.indexOf(':', p0 + 1);
    if (p0 > -1) {
        int p1 = s.indexOf(':', p0 + 1);
        if (p1 > -1) {

            int points[3];
            points[0] = s.indexOf('.', p0);
            int p2 = points[0] - 1;
            for (; p2 > p0; --p2) {
                if (!s[p2].isDigit())
                    break;
            }
            QString ip = s.mid(p2 + 1, p1 - p2 - 1);
            emit gotNewIp(ip);
        }
    }
}

void OpenvpnManager::cancel(const QString & msg)
{
    stop();
    emit blockOnDisconnect();
    emit sendError(msg);
}

void OpenvpnManager::stop()
{
    if (openvpnRunning()) {
        if (mSocket.get() != NULL) {
            if (mSocket->isOpen() && mSocket->isValid()) {
                if (mSocket->state() != QAbstractSocket::ConnectedState) {
                    Log::serviceLog("Cannot send signal SIGTERM due to disconnected socket");
                } else {
                    Log::serviceLog("signal SIGTERM");
                    //_soc->write("echo all\n");
                    //_soc->flush();

                    mSocket->write("signal SIGTERM\nexit\n");
                    mSocket->flush();
                    mSocket->close();
                }
            }
            disconnectFromOpenvpnSocket();

            for (int cn = 0; cn < 8 && openvpnRunning(); ++cn)
                QThread::msleep(100);       // just sleep now; without this delay it fails to jump
        }

        QThread::msleep(200);
        removeProcess();

        if (openvpnRunning())
            Log::serviceLog("Stop(): cannot soft stop OpenVPN process");
    }

    // This could be the case if openvpn stopped but the socket is still open
    if (mSocket.get() != NULL)
        disconnectFromOpenvpnSocket();

    stopObfsproxy();

    if (mFileSystemWatcher.get() != NULL) {       // OpenVPN log file watcher
        mFileSystemWatcher->removePath(ServicePathHelper::Instance()->openvpnLogFilename());
        delete mFileSystemWatcher.release();
    }

    setState(vpnStateDisconnected);
}

void OpenvpnManager::removeProcess()
{
    if (mProcess.get() != NULL) {
        QProcess * t = mProcess.release();
        t->deleteLater();
    }
}

void OpenvpnManager::logStderr()
{
    Log::serviceLog("ReadStderr(): " + mProcess->readAllStandardError());
}

void OpenvpnManager::logStdout()
{
    Log::serviceLog("ReadStdout(): " + mProcess->readAllStandardOutput());
}

void OpenvpnManager::processStateChanged(QProcess::ProcessState st)
{
    Log::serviceLog("OpenvpnManager::processStateChanged(): newState = " + QString::number(st));
    // TODO: handle open vpn  process startup
    setupFileWatcher();
}

void OpenvpnManager::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Log::serviceLog("OpenvpnManager::processFinished(): exitCode = " + QString::number(exitCode) + " exitStatus = " +  QString::number(exitStatus));
    // OpenVpn crashed or just spawn a child and exit during startup
    if (exitCode != 0) { // TODO: -1 handle open vpn process startup
        // TODO: -1 handle used socket
        // MANAGEMENT: Socket bind failed on local address [AF_INET]127.0.0.1:6842: Address already in use

// TODO: -0 delete or not? it spawns child and exits
//      RemoveProcess();
        if (!openvpnRunning())
            setState(vpnStateDisconnected);
    } else {
        setupFileWatcher();
        connectToOpenvpnSocket();
    }
}

void OpenvpnManager::setupFileWatcher()
{
    if (mFileSystemWatcher.get() == NULL) {
        QFile f(ServicePathHelper::Instance()->openvpnLogFilename());
        if (f.exists()) {
            mLogFilePosition = 0;       // OpenVpn will truncate

            mFileSystemWatcher.reset(new QFileSystemWatcher());
            mFileSystemWatcher->addPath(ServicePathHelper::Instance()->openvpnLogFilename());

            Log::serviceLog("Monitoring " + ServicePathHelper::Instance()->openvpnLogFilename());
            connect(mFileSystemWatcher.get(), SIGNAL(fileChanged(const QString &)),
                    this, SLOT(openvpnLogfileChanged(const QString &)));
        }
    }
}

void OpenvpnManager::disconnectFromOpenvpnSocket()
{
    if (mSocket.get() != NULL) {
        Log::serviceLog("disconnecting from openvpn management socket");

        disconnect(mSocket.get(), SIGNAL(error(QAbstractSocket::SocketError)),
                   this, SLOT(socketError(QAbstractSocket::SocketError)));
        disconnect(mSocket.get(), SIGNAL(readyRead()),
                   this, SLOT(socketReadyRead()));

        mSocket->abort();
        mSocket.release()->deleteLater();
        Log::serviceLog("done disconnecting from openvpn management socket");
    }
}

void OpenvpnManager::connectToOpenvpnSocket()
{
    Log::serviceLog("connecting to openvpn management socket");
    disconnectFromOpenvpnSocket();
    mSocket.reset(new QTcpSocket());
    connect(mSocket.get(), SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(mSocket.get(), SIGNAL(readyRead()),
            this, SLOT(socketReadyRead()));
    connect(mSocket.get(), SIGNAL(connected()),
            this, SLOT(socketConnected()));
    mSocket->connectToHost(kLocalAddress, mLocalPort.toInt());
    Log::serviceLog("done connecting to openvpn management socket");
}

void OpenvpnManager::socketError(QAbstractSocket::SocketError error)
{
    if (error == QAbstractSocket::RemoteHostClosedError) {
        Log::serviceLog("Disconnecting from openvpn management because remote closed");
        disconnectFromOpenvpnSocket();
        return;
    }

    Log::serviceLog("Error connecting to OpenVPN management socket" + QString::number(error));
    if (NULL != mSocket.get())
        mSocket.release()->deleteLater();
}

void OpenvpnManager::socketReadyRead()
{
    Log::serviceLog("socketReadyRead called()");
    if (!mSocket->canReadLine()) {
        Log::serviceLog("socketReadyRead socket line not ready");
        return;
    }

    QString s = mSocket->readAll();
    mSocket->flush();

    QStringList sl = s.split('\n', QString::SkipEmptyParts);
    for (int k = 0; k < sl.size(); ++k)
        parseSocketLine(sl[k].trimmed());
}

void OpenvpnManager::parseSocketLine(QString s)
{
    // TODO: -1 hash_map
    // TODO: -2 state machine

    if (s.startsWith('>')) {
        // >PASSWORD:Need 'Auth' username/password
        int p = s.indexOf(':');
        if (p > -1) {
            QString word = s.mid(1, p - 1);
            parseSocketQueryWord(word, s);
        } else {
            // without :
            // just ignore
        }
    } else {
        // not starting with >
        QString word;
        if (s[0].isDigit()) {
            // 1432065857,GET_CONFIG,,,
            int p[4];
            p[0] = s.indexOf(',', 1);
            p[1] = s.indexOf(',', p[0] + 1);
            if (p[1] > -1) {
                word = s.mid(p[0] + 1, p[1] - p[0] - 1);

                // check error
                // 1436967388,CONNECTED,ERROR,10.9.0.6,158.255.211.19
                int p3 = s.indexOf(',', p[1] + 1);
                bool err = false;
                if (p3 > -1) {
                    if (p3 > p[1] + 1) {
                        QString third = s.mid(p[1] + 1, p3 - p[1] - 1);
                        if (third.contains("ERROR", Qt::CaseInsensitive))
                            err = true;
                    }
                }
                if (!err)
                    parseSocketStateWord(word, s);
            } else {
                // TODO: -2
                word = s;
                parseSocketPlainWord(word, s);
            }
        } else {
            int p = s.indexOf(':');
            if (p > -1) {
                // SUCCESS: 'Auth' password entered, but not yet verified
                word = s.mid(0, p);
            } else {
                word = s;
            }
            parseSocketPlainWord(word, s);
        }
    }
}

void OpenvpnManager::parseSocketStateWord(const QString & word, const QString & s)
{
    // TODO: -1 hash_map
    // TODO: -2 state machine

    bool isnew = false;
    if (word != mPreviousStateWord) {
        Log::serviceLog(s);
        Log::serviceLog("processing state word '" + word + "'");
        isnew = true;
    }
    if (word.compare("CONNECTED", Qt::CaseInsensitive) == 0) {
        if (isnew)
            gotConnected(s);
        else {
//          Log::serviceLog("isnew = false; word = " + word + " _prev_st_word = " + _prev_st_word);
        }
    } else if (word.compare("CONNECTING", Qt::CaseInsensitive) == 0 ||
               word.compare("TCP_CONNECT", Qt::CaseInsensitive) == 0) {
        setState(vpnStateConnecting);
        emit stateWordChanged(ovnStateConnecting);
    } else if (word.compare("WAIT", Qt::CaseInsensitive) == 0) {
        emit stateWordChanged(ovnStateWait);
//  } else { if (word.compare("AUTH", Qt::CaseInsensitive) == 0) {
//      emit stateWordChanged(word);
    } else if (word.compare("EXITING", Qt::CaseInsensitive) == 0) {
        if (mReconnect) {
            // initiate autoreconnect
            startReconnectTimer();
            setState(vpnStateConnecting);
        } else {
            setState(vpnStateDisconnected);
        }
    } else if (word.compare("RECONNECTING", Qt::CaseInsensitive) == 0) {
        setState(vpnStateConnecting);
        emit stateWordChanged(ovnStateReconnecting);
    } else if (word.compare("AUTH", Qt::CaseInsensitive) == 0) {
        setState(vpnStateConnecting);
        emit stateWordChanged(ovnStateAuth);
    } else if (word.compare("GET_CONFIG", Qt::CaseInsensitive) == 0) {
        emit stateWordChanged(ovnStateGetConfig);
    } else if (word.compare("ASSIGN_IP", Qt::CaseInsensitive) == 0) {
        setState(vpnStateConnecting);
        emit stateWordChanged(ovnStateAssignIP);
    } else if (word.compare("TCP_CONNECT'", Qt::CaseInsensitive) == 0) {
        emit stateWordChanged(ovnStateTCPConnecting);
    } else if (word.compare("RESOLVE", Qt::CaseInsensitive) == 0) {
        emit stateWordChanged(ovnStateResolve);
        if (mNetDown) {
            stop();
            emit sendError("Turn Internet connection on manually, please");
        }
    } else {
        Log::serviceLog("Got unknown state word " + word);
        emit stateWordChanged(ovnStateUnknown);
    }
    mPreviousStateWord = word;
}

void OpenvpnManager::parseSocketPlainWord(const QString & word, const QString & s)
{
    // TODO: -1 hash_map
    // TODO: -2 state machine
    if (word.compare("END", Qt::CaseInsensitive) == 0) {
        ;
    } else {
        Log::serviceLog(s);
        Log::serviceLog("processing plain word '" + word + "'");
        if (word.compare("SUCCESS", Qt::CaseInsensitive) == 0) {
            ;
        } else {
            if (word.compare("ERROR", Qt::CaseInsensitive) == 0 || word.compare("FATAL", Qt::CaseInsensitive) == 0) {
                disconnectFromOpenvpnSocket();
                emit blockOnDisconnect();
                emit sendError(s);
            }
        }
    }
}

void OpenvpnManager::parseSocketQueryWord(const QString & word, const QString & s)
{
    // TODO: -1 hash_map
    // TODO: -2 state machine

    Log::serviceLog("ProcessRtWord(): '"  + s + "'");
    Log::serviceLog("processing RT word '" + word + "'");

    if (word.compare("INFO", Qt::CaseInsensitive) == 0) {
        // INFO:
        // just ignore
    } else if (word.compare("HOLD", Qt::CaseInsensitive) == 0) {
        if (s.indexOf("hold release") > -1) {
            Log::serviceLog("hold off");
            mSocket->write("hold off\n");
            Log::serviceLog("hold release");
            mSocket->write("hold release\n");
            mSocket->flush();
        }
    } else if (word.compare("PASSWORD", Qt::CaseInsensitive) == 0) {
        if (s.indexOf("Need 'Auth' username/password") > -1) {
            Log::serviceLog("sending vpn login+password");
            QString u1 = "username \"Auth\" \"" + EscapePsw(mVPNUsername) + "\"\n";
            QString p1 = "password \"Auth\" \"" + EscapePsw(mVPNPassword) + "\"\n";
            mSocket->write(u1.toLatin1());
            mSocket->write(p1.toLatin1());
            mSocket->flush();
        } else {
            int p = s.indexOf(':');
            if (s.indexOf("Verification Failed", p, Qt::CaseInsensitive) > -1) {
                mError = true;
                // OpenVpn exiting
                showErrorMessageCleanup(s.mid(p + 1));
            }
        }
    } else if (word.compare("SUCCES", Qt::CaseInsensitive) == 0) {
        ;
    } else {
        if (word.compare("FATAL", Qt::CaseInsensitive) == 0) {
            int p = s.indexOf(':');
            QString msg = s.mid(p + 1);
            if (msg.compare("Cannot allocate TUN/TAP dev dynamically", Qt::CaseInsensitive) == 0) {
                gotTunErr(msg);
            } else {
                mError = true;
                cancel(msg);
            }
        }
    }
}

void OpenvpnManager::showErrorMessageCleanup(QString msg)
{
    disconnectFromOpenvpnSocket();
    removeProcess();

    setState(vpnStateDisconnected);
    emit sendError(msg);
}

bool OpenvpnManager::openvpnRunning()
{
    bool running = false;

    running = mProcess.get() != NULL &&
              (mProcess->state() == QProcess::Running ||
               mProcess->state() == QProcess::Starting);

    if (!running)
        running = mSocket.get() != NULL && mSocket->isOpen();

//  if (!is)        // lookup child
    {
#ifdef WIN32
        QString result = OsSpecific::runCommandFast("tasklist.exe", QStringList()<<"/svc");
        if (result.contains("openvpn.exe") || result.contains("ovpntray.exe")) {
            running = true;
        }
#else

        /*  command composition '|' does not work here
            QString sout = OsSpecific::Instance()->RunFastCmd(OsSpecific::Instance()->IsRunningCmd());
            QString s3 = sout.trimmed();
            int p = s3.indexOf(' ');
            QString s4 = s3.mid(0, p);
            bool converted;
            _pid = s4.toInt(&converted);
            if (converted)
            {
                if (_pid > 0)
                    is = true;
            }
            return is;
        */

#ifdef Q_OS_DARWIN
        QString result = OsSpecific::instance()->runCommandFast(ServicePathHelper::Instance()->openvpnRunningScriptFilename());
        if (result.trimmed() != "0") {
            running = true;
        }
#else
        if (!running) {
            QTemporaryFile file(QDir::tempPath() + "/safejumper-tmp-XXXXXX.sh");
            QTemporaryFile outf(QDir::tempPath() + "/safejumper-tmp-XXXXXX.out");
            if (file.open())
                if (outf.open()) {
                    QString script = QString("ps -xa | grep open | grep execut | grep Safeju > ") + outf.fileName();
                    file.write(script.toLatin1());
                    file.flush();

                    int re = QProcess::execute("/bin/bash", QStringList() << file.fileName());
                    switch (re) {
                    case -2:
                        Log::serviceLog("openvpnRunning(): -2 the process cannot be started");
                        break;
                    case -1:
                        Log::serviceLog("openvpnRunning(): -1 the process crashed");
                        break;
                    case 0: {
                        QByteArray ba = outf.readAll();
                        QString s2(ba);
                        QString s3 = s2.trimmed();
                        int p = s3.indexOf(' ');
                        QString s4 = s3.mid(0, p);
                        bool converted;
                        mPID = s4.toInt(&converted);
                        if (converted) {
                            if (mPID > 0) {
                                running = true;
                                //                              AttachMgmt();
                            }
                        }
                        break;
                    }
                    case 1:
                        running = false;     // no lines
                        break;
                    case 2:
                        running = false;     // grep failure
                        break;
                    default:
                        Log::serviceLog("IsOvRunning(): ps-grep return code = " + QString::number(re));
                        break;
                    }
                }
        }
#endif
#endif  // else WIN32
    }
    Log::serviceLog(QString("openvpnRunning return value:") + QString(running ? "true": "false") );
    return running;
}

void OpenvpnManager::netDown(bool down)
{
    try {
#ifndef Q_OS_WIN
        if (down) {
            setRights();
            Log::serviceLog("NetDown()");
            QString ss = OsSpecific::instance()->runCommandFast(ServicePathHelper::Instance()->netDownFilename());
            mNetDown = true;
            if (!ss.isEmpty())
                Log::serviceLog(ss);
        } else {
            // TODO: need netup on macos and linux
        }
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
            Log::serviceLog("Cannot enumerate adapters, error code: " + QString::number(r));
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
                    std::wstring wa;
                    if (down)
                        wa = L"interface set interface name=\"" + std::wstring(p->FriendlyName) + L"\" admin=disable";
                    else
                        wa = L"interface set interface name=\"" + std::wstring(p->FriendlyName) + L"\" admin=enable";
                    Log::serviceLog("ShellExecute '" + QString::fromStdWString(prog) + "' , Args: '" + QString::fromStdWString(wa) + "'");
                    HINSTANCE hi = ShellExecute(NULL, NULL, prog.c_str(), (LPWSTR)wa.c_str(), NULL, SW_HIDE);	// already admin
                    if ((int)hi <= 32)
                        Log::serviceLog("Cannot ShellExecute hi = " + QString::number((int)hi) + " err code = " + QString::number(GetLastError()));
                    else {
                        Log::serviceLog("ShellExecute OK");
                        mNetDown = true;
                    }
                }
            }
        }
#endif	// Q_OS_WIN
    } catch(std::exception & ex) {
        Log::serviceLog(ex.what());
    }
}

void OpenvpnManager::killRunningOpenvpn()
{
    Log::serviceLog(QString("KillRunningOV() enter"));
    if (openvpnRunning()) {
        connectToOpenvpnSocket();
        if (mSocket.get())
            for (int count = 0; count < 8 && !mSocket->isOpen() && !mSocket->isValid(); ++count)
                QThread::msleep(100);       // HACK: -0 just timeout instead of waiting for connection;

        stop();     // soft stop

        if (openvpnRunning()) {
            // TODO: -0 kill -9 prog

            if (mPID > 0) {
                QStringList a;
                a << "-9" << QString::number(mPID);
                try {
                    QProcess::execute("/bin/kill", a);
                } catch(std::exception & ex) {
                    Log::serviceLog(ex.what());
                    emit sendError(ex.what());
                }
            } else {
#ifdef Q_OS_WIN
                // Unknown openvpn process, kill with taskkill
                OsSpecific::runCommandFast("taskkill", QStringList() << "/F" << "/IM" << "openvpn.exe");
                OsSpecific::runCommandFast("taskkill", QStringList() << "/F" << "/IM" << "ovpntray.exe");
#endif
            }
        }
    }
    Log::serviceLog(QString("KillRunningOV() exit"));
}

void OpenvpnManager::socketConnected()
{
    Log::serviceLog("Socket to openvpn connected");
}

void OpenvpnManager::startReconnectTimer()
{
    QTimer::singleShot(1000, this, SLOT(reconnectTimeout()));
}

void OpenvpnManager::reconnectTimeout()
{
    ++mReconnectAttempt;
    if (openvpnRunning()) {
        if (mReconnectAttempt < G_Max_Reconnect)
            QTimer::singleShot(200, this, SLOT(reconnectTimeout()));
        else {
            stop();
            startConnecting();    // force stop then start
        }
    } else {
        startConnecting();
    }
}

void OpenvpnManager::stopObfsproxy()
{
    if (obfsproxyRunning()) {
        Log::serviceLog("Obfsproxy is running, so stopping it");
#ifdef Q_OS_WIN
        // Unknown openvpn process, kill with taskkill
        OsSpecific::runCommandFast("taskkill", QStringList() << "/F" << "/IM" << "obfsproxy.exe");
#else
        if (mObfsproxy.get()) {
            Log::serviceLog("We have a pointer to obfsproxy process, so telling it to terminate then kill");
            mObfsproxy->terminate();
            mObfsproxy->kill();
            mObfsproxy.release()->deleteLater();
            while (obfsproxyRunning()) { // Make sure it stopped so it will start
                Log::serviceLog("Waiting for obfsproxy to terminate");
                QThread::sleep(100);
            }
        } else {
            Log::serviceLog("No pointer to obfsproxy process, so killing it");
            // We don't have a pointer to the running obfsproxy, so use taskkill or killall
            OsSpecific::runCommandFast("killall", QStringList() << "obfsproxy");
        }
#endif
    }
}

void OpenvpnManager::runObfsproxy(const QString &srv,
                                  const QString &port,
                                  const QString &obfstype,
                                  const QString & local_port)
{
    Log::serviceLog("runObfsproxy called with server " + srv + " and type " + obfstype);

    Log::serviceLog("SRV = " + srv);

#ifndef Q_OS_WIN
    if (!obfsproxyRunning()) {
#endif
        QStringList args;
        args << ServicePathHelper::Instance()->obfsproxyFilename();

#ifndef Q_OS_WIN
        args << "--log-file " + ServicePathHelper::Instance()->obfsproxyLogFilename();
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
        mObfsproxy->setStandardErrorFile(ServicePathHelper::Instance()->obfsproxyLogFilename());
        mObfsproxy->setStandardOutputFile(ServicePathHelper::Instance()->obfsproxyLogFilename());
        Log::serviceLog("Executing obfsproxy with command " + cmd);
        mObfsproxy->start(cmd);
        QThread::msleep(100);
        Log::serviceLog(QString("if (!IsObfsRunning()) ") + cmd);
#ifndef Q_OS_WIN
    }
#endif
    Log::serviceLog("runObfsproxy() done");
}

void OpenvpnManager::obfsFinished(int exitCode, QProcess::ExitStatus status)
{
    Log::serviceLog("obfsFinished with code " + QString::number(exitCode) +
                    " and status " + QString::number(status));
    mObfsproxy.release()->deleteLater();

    if (mState == vpnStateConnecting)
        launchObfsproxy();
}

bool OpenvpnManager::obfsproxyRunning()
{
    bool b = false;
#ifdef Q_OS_WIN
    QString result = OsSpecific::runCommandFast("tasklist.exe", QStringList()<<"/svc");
    if (result.contains("obfsproxy.exe")) {
        Log::serviceLog("obfsproxy is running");
        b = true;
    }
#else
    {
        QString s1 = OsSpecific::instance()->runCommandFast("ps ax");
        b = s1.contains("/obfsproxy");
        QString result = b ? "true" : "false";
        Log::serviceLog("IsObfsRunning result: " + result);
    }
#endif
    return b;
}

void OpenvpnManager::setRights()
{
    // Qt uses permissions in HEX while OS's in 8-based !


    // will ask for elevated rights inside
#ifdef Q_OS_DARWIN

    setChmod("0744", ServicePathHelper::Instance()->upScriptFilename());
    setChown(ServicePathHelper::Instance()->upScriptFilename());
    setChmod("0744", ServicePathHelper::Instance()->downScriptFilename());
    setChown(ServicePathHelper::Instance()->downScriptFilename());

    setChmod("0755", ServicePathHelper::Instance()->openvpnFilename());
    setChown(ServicePathHelper::Instance()->openvpnFilename());

    setChmod("04555", ServicePathHelper::Instance()->netDownFilename());
    setChown(ServicePathHelper::Instance()->netDownFilename());

    setChmod("04555", ServicePathHelper::Instance()->launchopenvpnFilename());
    setChown(ServicePathHelper::Instance()->launchopenvpnFilename());

    system(QString("touch %1").arg(ServicePathHelper::Instance()->openvpnLogFilename()).toStdString().c_str());
    // Don't chmod the openvpn log file when running safechecker
    // Prevents prompt for authorization after every error
#ifndef SAFECHECKER
    setChmod("777", ServicePathHelper::Instance()->openvpnLogFilename());
#endif // SAFECHECKER

#endif		// Q_OS_DARWIN

#ifdef Q_OS_LINUX
    setChown(ServicePathHelper::Instance()->launchopenvpnFilename());
    setChmod("777", ServicePathHelper::Instance()->launchopenvpnFilename());				// odrer is important

    setChown(ServicePathHelper::Instance()->netDownFilename());
    setChmod("777", ServicePathHelper::Instance()->netDownFilename());
#endif

    //ReleaseRights();

    QThread::msleep(200);		// HACK: -1 wait till file system changes become effective
}

void OpenvpnManager::setChmod(const char * sflags, const QString & pfn)
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
        QProcess::execute("/bin/chmod", args1);				// mac, ubuntu

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

void OpenvpnManager::setChown(const QString & pfn)
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
            QProcess::execute("/usr/bin/chgrp", args);			// Mac
        }

        {
            QStringList args1;
            args1 << "root" << pfn;
            QProcess::execute("/usr/sbin/chown", args1);		// Mac
        }
#else
        {
            QStringList args1;
            args1 << "0:0" << pfn;
            QProcess::execute("/bin/chown", args1);		// ubuntu
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

void OpenvpnManager::setIPv6(bool enable)
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
        Log::serviceLog("Changing IPv6 state to: " + QString(enable ? "enabled" : "disabled"));
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
                val = old & ( ~((DWORD)0xFF));
            else
                val = old | 0xFF;
            lRes = ::RegSetValueEx(hKey, gs_regname, 0, REG_DWORD, reinterpret_cast<LPBYTE>(&val), sz);
            if (ERROR_SUCCESS != lRes)
                throw std::runtime_error(("IPv6 disabling failure code: " + QString::number(lRes)).toStdString().c_str());
        }
        /*
                static const char * en = "netsh interface ipv6 set teredo client";
                static const char * dis = "netsh interface ipv6 set teredo disabled";
                const char * s = enable ? en : dis;
                int res = QProcess::execute(s);
                Log::serviceLog("IPv6 change state command return code: " + QString::number(res));
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
            QProcess::execute(gs_ns, a);
        }
        {
            QStringList a;
            a << ac << gs_ether;
            QProcess::execute(gs_ns, a);
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

bool OpenvpnManager::IPv6()
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
                    if ( (val & 0xFF) == 0xFF)	// 0x01 to disable IPv6 on all tunnel interfaces https://support.microsoft.com/en-us/kb/929852
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
void OpenvpnManager::enableTap()
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
        Log::serviceLog("Cannot enumerate adapters, error code: " + QString::number(r));
    } else {
        for (; p != NULL; p = p->Next) {
            QString s = QString::fromWCharArray(p->Description);
            if (s.indexOf("TAP-Windows") > -1) {
                std::wstring prog = L"netsh";
                std::wstring wa = L"interface set interface name=\"" + std::wstring(p->FriendlyName) + L"\" admin=enabled";
                Log::serviceLog("ShellExecute '" + QString::fromStdWString(prog) + "' , Args: '" + QString::fromStdWString(wa) + "'");
                HINSTANCE hi = ShellExecute(NULL, NULL, prog.c_str(), (LPWSTR)wa.c_str(), NULL, SW_HIDE);	// already admin
                if ((int)hi <= 32)
                    Log::serviceLog("Cannot ShellExecute hi = " + QString::number((int)hi) + " err code = " + QString::number(GetLastError()));
                else
                    Log::serviceLog("ShellExecute OK");
            }
        }
    }
}
#endif	// Q_OS_WIN
