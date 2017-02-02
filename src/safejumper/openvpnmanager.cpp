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

#include <QFile>
#include <QDir>
#include <stdexcept>

#include "authmanager.h"
#include "osspecific.h"
#include "scr_map.h"
#include "wndmanager.h"
#include "setting.h"
#include "log.h"
#include "loginwindow.h"
#include "pathhelper.h"

const QString kLocalAddress = "127.0.0.1";

OpenvpnManager::OpenvpnManager()
    : mState(ovsDisconnected),
      mPID(0),
      mSocket(0),
      mStateTimer(NULL)
{}

OpenvpnManager::~OpenvpnManager()
{

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

OpenvpnManager::OvState OpenvpnManager::state()
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

void OpenvpnManager::startWithServer(size_t srv)
{
    Scr_Map::Instance()->SetServer(srv);
    start();
}

void OpenvpnManager::start()
{
    WndManager::Instance()->ClosePortDlg();
    mPortDialogShown = false;
    mInPortLoop = false;    // TODO: -0
    mReconnectAttempt = 0;

    launchOpenvpn();
}

void OpenvpnManager::launchOpenvpn()
{
    stop();
    // TODO: -1 cleanup

    if (AuthManager::Instance()->loggedIn()) {
        setState(ovsConnecting);
        int enc = Setting::Encryption();
        bool obfs = enc == ENCRYPTION_OBFS_TOR;
        QString server = Setting::Instance()->Server();
        QString port = Setting::Instance()->Port();
        if (server.isEmpty() || port.isEmpty()) {
            QString message = "Server or port is empty, select a location";
            log::logt(message);
            WndManager::Instance()->ErrMsg(message);
            return;
        }
        if (obfs) {
            OsSpecific::Instance()->runObfsproxy(server, port, "1050");
            if (!OsSpecific::Instance()->obfsproxyRunning()) {
                log::logt("Cannot run Obfsproxy");
                WndManager::Instance()->ErrMsg("Cannot run Obfsproxy");
                return;
            }
        }
        try {
            OsSpecific::Instance()->SetIPv6(!Setting::Instance()->IsDisableIPv6());
#ifdef Q_OS_WIN
            OsSpecific::Instance()->EnableTap(); // TODO check win10 tap
#endif
        } catch(std::exception & ex) {
            log::logt(ex.what());
            if (Setting::Instance()->IsDisableIPv6()) {
                WndManager::Instance()->ErrMsg(QString("Cannot disable IPv6 ") + ex.what());
                return;
            }
        }

//      AuthManager::Instance()->SetNewIp("");
        mTunError = false;
        mError = false;
        mProcessing = false;

#ifndef NO_PARAMFILE
        if (!writeConfigFile()) {
            return; // Error, so return
        }
#endif  // NO_PARAMFILE

        QStringList args = getOpenvpnArgs();
        if (Setting::Instance()->IsFixDns() ||
                !Setting::Instance()->Dns1().isEmpty() ||
                !Setting::Instance()->Dns2().isEmpty())
            OsSpecific::Instance()->FixDnsLeak();

        QString prog = PathHelper::Instance()->openvpnFilename();
        log::logt("Prog is: " + prog);
        QString params = args.join(' ');
        log::logt("Args are:" + params);

        bool ok = false;
        try {
#ifndef Q_OS_WIN
            mParametersTempFile.reset(new QTemporaryFile());
            if (!mParametersTempFile->open())
                throw std::runtime_error("Cannot create tmp file.");

            OsSpecific::Instance()->SetRights();        // lean inside, throw on error

            mParametersTempFile->write(params.toLatin1());
            mParametersTempFile->flush();

            QStringList arg3;
            arg3 << mParametersTempFile->fileName();
            mParametersTempFile->close();

//#ifdef Q_OS_MAC
//          log::logt("######  touch launcher ####");
//          OsSpecific::Instance()->RunFastCmd("touch -a " + PathHelper::Instance()->LauncherPfn());
//#endif
            log::logt("######  before exec ####");

#ifdef Q_OS_LINUX
            OsSpecific::Instance()->ExecAsRoot(PathHelper::Instance()->launchopenvpnFilename(), arg3);
#else
//#ifdef Q_OS_MAC
//            OsSpecific::Instance()->ExecAsRoot(prog, args);     // force password dialog; without launcher
//#else
            int r3 = QProcess::execute(PathHelper::Instance()->launchopenvpnFilename(), arg3);    // 30ms block internally
            log::logt("Launched " + PathHelper::Instance()->launchopenvpnFilename() + " with argument " + arg3.at(0));
            log::logt("QProcess::execute() returns " + QString::number(r3));
            log::logt("###############");
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

            log::logt("before attaching to OpenVPN");
            connectToOpenvpnSocket();   // TODO: -1 wait for slow starting cases
            setupFileWatcher();
            log::logt("after attaching to OpenVPN");
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
            log::logt("Process ID is: " + QString::number(mProcess->processId()));
#endif
            ok = true;
        } catch(std::exception & ex) {
            log::logt(ex.what());
            WndManager::Instance()->ErrMsg(ex.what());
        }

        if (mParametersTempFile.get())
            delete mParametersTempFile.release();

        if (ok) {
            log::logt("openvpn launched, starting timer");
            mStartDateTime = QDateTime::currentDateTimeUtc().toTime_t();
            startTimer();
        } else
            setState(ovsDisconnected);

    }
    log::logt("launchopenvpn done");
#undef NO_PARAMFILE
}

void OpenvpnManager::processError(QProcess::ProcessError error)
{
    log::logt("OpenvpnManager::processError(): error = " + QString::number(error));
    WndManager::Instance()->HandleDisconnected();
}

void OpenvpnManager::processStarted()
{
    log::logt("OpenvpnManager::ConnectStarted()");
}

bool OpenvpnManager::writeConfigFile()
{
    int enc = Setting::Encryption();
    bool obfs = enc == ENCRYPTION_OBFS_TOR;
    QFile ff(PathHelper::Instance()->openvpnConfigFilename());
    if (!ff.open(QIODevice::WriteOnly)) {
        QString se = "Cannot write config file '" + PathHelper::Instance()->openvpnConfigFilename() + "'";
        log::logt(se);
        WndManager::Instance()->ErrMsg(se);
        return false;
    }
    ff.write("client\n");
    ff.write("dev tun\n");
    ff.write("proto ");
    ff.write(Setting::Instance()->Protocol().toLatin1());
    ff.write("\n");       // "tcp"/"udp"
//ff.write("proto udp\n");
//ff.write("proto tcp\n");

    QString server = Setting::Instance()->Server();
    QString port = Setting::Instance()->Port();
    if (server.isEmpty() || port.isEmpty()) {
        QString message = "Server or port is empty, select a location";
        log::logt(message);
        WndManager::Instance()->ErrMsg(message);
        return false;
    }
    ff.write("remote ");
    ff.write(server.toLatin1());
    ff.write(" ");
    ff.write(port.toLatin1());
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
    ff.write("verb 5\n");
    ff.write("reneg-sec 0\n");
    ff.write("route-method exe\n");
    ff.write("route-delay 2 0\n");

    if (enc == ENCRYPTION_ECC || enc == ENCRYPTION_ECCXOR) {
//          ff.write("tls-cipher ECDHE-ECDSA-AES256-GCM-SHA384\n");
        ff.write("tls-cipher TLS-ECDHE-ECDSA-WITH-AES-256-GCM-SHA384\n");

        //ff.write("tls-cipher ECDH\n");
        //ff.write("tls-cipher !ECDH\n");

        ff.write("ecdh-curve secp384r1\n");

        if (enc == ENCRYPTION_ECCXOR)
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

#ifdef Q_OS_WIN
    if (Setting::Instance()->IsFixDns()) {
        if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS10)
            ff.write("block-outside-dns\n");
    }
#endif

//ff.write("dhcp-option DNS 146.185.134.104\n");
//ff.write("dhcp-option DNS 146.185.134.104\n");

    if (obfs) {
// TODO: -0 OS
        ff.write("socks-proxy 127.0.0.1 1050\n");
        ff.write("route ");
        ff.write(Setting::Instance()->Server().toLatin1());
        ff.write(" 255.255.255.255 net_gateway\n");
    }

    ff.flush();
    ff.close();
    return true;
}

QStringList OpenvpnManager::getOpenvpnArgs()
{
    QStringList args;
    int enc = Setting::Encryption();
//          << "--auth-nocache"
#ifndef NO_PARAMFILE
    args << "--config" << PathHelper::Instance()->openvpnConfigFilename(); // /tmp/proxysh.ovpn
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
            << "--remote" << Setting::Instance()->Server() << Setting::Instance()->Port()

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
    args << "--management" << kLocalAddress << Setting::Instance()->LocalPort()
         << "--management-hold"
         << "--management-query-passwords";
    args << "--log" << PathHelper::Instance()->openvpnLogFilename();           // /tmp/openvpn.log

//          << "--script-security" << "3" << "system"
//          << "--script-security" << "2" << "execve"       // https://openvpn.net/index.php/open-source/documentation/manuals/69-openvpn-21.html
    args << "--script-security" << "3";

#ifndef Q_OS_WIN            // TODO: -0 DNS on linux
    args << "--up" << PathHelper::Instance()->upScriptFilename();     // /Applications/Safejumper.app/Contents/Resources/client.up.safejumper.sh
    args << "--down" << PathHelper::Instance()->downScriptFilename(); // /Applications/Safejumper.app/Contents/Resources/client.down.safejumper.sh
#endif
    args << "--up-restart";

    // TODO: -1 download cert from proxy.sh
    if (enc != ENCRYPTION_ECC && enc != ENCRYPTION_ECCXOR)
        args << "--ca" << PathHelper::Instance()->proxyshCaCertFilename();    // /tmp/proxysh.crt

    if (!Setting::Instance()->Dns1().isEmpty())
        args << "--dhcp-option" << "DNS" << Setting::Instance()->Dns1();
    if (!Setting::Instance()->Dns2().isEmpty())
        args << "--dhcp-option" << "DNS" << Setting::Instance()->Dns2();
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
    log::logt("checkState called");
    if (openvpnRunning()) {
        if (mSocket.get() == NULL) {
            connectToOpenvpnSocket();
        } else {
            if (mSocket->isOpen() && mSocket->isValid()) {
                if (mSocket->state() == QAbstractSocket::ConnectedState) {
                    mSocket->write("state\n");
                    mSocket->flush();
                }
            }
        }
    } else if (state() == ovsConnected) {
        // handle crush
        if (Setting::Instance()->IsReconnect())
            start();
        else
            setState(ovsDisconnected);
    }

    if (state() == ovsConnecting) {
        uint d = QDateTime::currentDateTimeUtc().toTime_t() - mStartDateTime;
        if (!mPortDialogShown && !mInPortLoop) {
            if (d > G_Delay_PortQuestion) {
                mPortDialogShown = true;
                WndManager::Instance()->ShowPortDlg();
            }
        }

        if (mInPortLoop) {
            if (d > G_Delay_PortIteration)
                tryNextPort();
        }
    }
    log::logt("checkState done");
}

void OpenvpnManager::openvpnLogfileChanged(const QString & pfn)
{
    if (mProcessing || mError)
        return;
    mProcessing = true;
    if (pfn == PathHelper::Instance()->openvpnLogFilename()) {
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

    log::logt("OpenVPNlogfile: " + s);
    if (s.contains("TCPv4_CLIENT link remote:", Qt::CaseInsensitive)) {
        parseNewIp(s);
    } else if (s.contains("Initialization Sequence Completed:", Qt::CaseInsensitive)) {
        gotConnected(s);
    } else if (s.contains("Opening utun (connect(AF_SYS_CONTROL)): Operation not permitted", Qt::CaseInsensitive)) {
        gotTunErr(s);
    }
}

void OpenvpnManager::setState(OvState st)
{
    if (st != mState) {
        mState = st;
        log::logt("Set state " + QString::number(st));
        switch (st) {
        case ovsConnected: {
            WndManager::Instance()->HandleConnected();
            break;
        }
        case ovsConnecting: {
            WndManager::Instance()->HandleConnecting();
            break;
        }
        case ovsDisconnected: {
            if (mStateTimer != NULL) {
                mStateTimer->stop();
                delete mStateTimer;
                mStateTimer = NULL;
            }
            WndManager::Instance()->HandleDisconnected();
            break;
        }
        default:
            break;
        }
    }
}

void OpenvpnManager::gotConnected(const QString & s)
{
    setState(ovsConnected);
    // extract IP
    //1432176303,CONNECTED,SUCCESS,10.14.0.6,91.219.237.159
    // 1460435651,CONNECTED,SUCCESS,10.200.1.6,85.236.153.236,465,192.168.58.170,35331
    int p = -1;
    for (int k = 0; k < 4; ++k)
        p = s.indexOf(',', p + 1);
    if (p > -1) {
        int p1 = s.indexOf(',', p + 1);
        QString ip = p1 > -1 ? s.mid(p + 1, p1 - p - 1) : s.mid(p + 1);
        if (Setting::Encryption() != ENCRYPTION_OBFS_TOR)   // for proxy it shows 127.0.0.1
            emit gotNewIp(ip);
    }

    AuthManager::Instance()->ForwardPorts();
}

void OpenvpnManager::gotTunErr(const QString & s)
{
    if (!mTunError && !mError) {
        mTunError = true;
        mError = true;
        log::logt("Got TUN Allocation error, reconnecting");
        start();
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
    LoginWindow::Instance()->BlockOnDisconnect();
    WndManager::Instance()->ErrMsg(msg);
}

void OpenvpnManager::stop()
{
    if (openvpnRunning()) {
        if (mSocket.get() != NULL) {
            if (mSocket->isOpen() && mSocket->isValid()) {
                if (mSocket->state() != QAbstractSocket::ConnectedState) {
                    log::logt("Cannot send signal SIGTERM due to disconnected socket");
                } else {
                    log::logt("signal SIGTERM");
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
            log::logt("Stop(): cannot soft stop OpenVPN process");
    }

    // This could be the case if openvpn stopped but the socket is still open
    if (mSocket.get() != NULL)
        disconnectFromOpenvpnSocket();

    if (OsSpecific::Instance()->obfsproxyRunning()) {
        OsSpecific::Instance()->StopObfs();
    }

    if (mFileSystemWatcher.get() != NULL) {       // OpenVPN log file watcher
        mFileSystemWatcher->removePath(PathHelper::Instance()->openvpnLogFilename());
        delete mFileSystemWatcher.release();
    }

    setState(ovsDisconnected);
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
    log::logt("ReadStderr(): " + mProcess->readAllStandardError());
}

void OpenvpnManager::logStdout()
{
    log::logt("ReadStdout(): " + mProcess->readAllStandardOutput());
}

void OpenvpnManager::processStateChanged(QProcess::ProcessState st)
{
    log::logt("OpenvpnManager::processStateChanged(): newState = " + QString::number(st));
    // TODO: handle open vpn  process startup
    setupFileWatcher();
}

void OpenvpnManager::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    log::logt("OpenvpnManager::processFinished(): exitCode = " + QString::number(exitCode) + " exitStatus = " +  QString::number(exitStatus));
    // OpenVpn crashed or just spawn a child and exit during startup
    if (exitCode != 0) { // TODO: -1 handle open vpn process startup
        // TODO: -1 handle used socket
        // MANAGEMENT: Socket bind failed on local address [AF_INET]127.0.0.1:6842: Address already in use

// TODO: -0 delete or not? it spawns child and exits
//      RemoveProcess();
        if (!openvpnRunning())
            setState(ovsDisconnected);
    } else {
        setupFileWatcher();
        connectToOpenvpnSocket();
    }
}

void OpenvpnManager::setupFileWatcher()
{
    if (mFileSystemWatcher.get() == NULL) {
        QFile f(PathHelper::Instance()->openvpnLogFilename());
        if (f.exists()) {
            mLogFilePosition = 0;       // OpenVpn will truncate

            mFileSystemWatcher.reset(new QFileSystemWatcher());
            mFileSystemWatcher->addPath(PathHelper::Instance()->openvpnLogFilename());

            log::logt("Monitoring " + PathHelper::Instance()->openvpnLogFilename());
            connect(mFileSystemWatcher.get(), SIGNAL(fileChanged(const QString &)),
                    this, SLOT(openvpnLogfileChanged(const QString &)));
        }
    }
}

void OpenvpnManager::disconnectFromOpenvpnSocket()
{
    if (mSocket.get() != NULL) {
        log::logt("disconnecting from openvpn management socket");

        disconnect(mSocket.get(), SIGNAL(error(QAbstractSocket::SocketError)),
                   this, SLOT(socketError(QAbstractSocket::SocketError)));
        disconnect(mSocket.get(), SIGNAL(readyRead()),
                   this, SLOT(socketReadyRead()));

        mSocket->abort();
        mSocket.release()->deleteLater();
        log::logt("done disconnecting from openvpn management socket");
    }
}

void OpenvpnManager::connectToOpenvpnSocket()
{
    log::logt("connecting to openvpn management socket");
    disconnectFromOpenvpnSocket();
    mSocket.reset(new QTcpSocket());
    connect(mSocket.get(), SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(mSocket.get(), SIGNAL(readyRead()),
            this, SLOT(socketReadyRead()));
    connect(mSocket.get(), SIGNAL(connected()),
            this, SLOT(socketConnected()));
    mSocket->connectToHost(kLocalAddress, Setting::Instance()->LocalPort().toInt());
    log::logt("done connecting to openvpn management socket");
}

void OpenvpnManager::socketError(QAbstractSocket::SocketError error)
{
    if (error == QAbstractSocket::RemoteHostClosedError) {
        disconnectFromOpenvpnSocket();
        return;
    }

    log::logt("Error connecting to OpenVPN management socket" + QString::number(error));
    if (NULL != mSocket.get())
        mSocket.release()->deleteLater();
}

void OpenvpnManager::socketReadyRead()
{
    log::logt("socketReadyRead called()");
    if (!mSocket->canReadLine()) {
        log::logt("socketReadyRead socket line not ready");
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
        log::logt(s);
        log::logt("processing state word '" + word + "'");
        isnew = true;
    }
    if (word.compare("CONNECTED", Qt::CaseInsensitive) == 0) {
        if (isnew)
            gotConnected(s);
        else {
//          log::logt("isnew = false; word = " + word + " _prev_st_word = " + _prev_st_word);
        }
    } else if (word.compare("CONNECTING", Qt::CaseInsensitive) == 0) {
        setState(ovsConnecting);
        WndManager::Instance()->HandleState(word);
    } else if (word.compare("WAIT", Qt::CaseInsensitive) == 0) {
        WndManager::Instance()->HandleState(word);
//  } else { if (word.compare("AUTH", Qt::CaseInsensitive) == 0) {
//      WndManager::Instance()->HandleState(word);
    } else if (word.compare("EXITING", Qt::CaseInsensitive) == 0) {
        if (Setting::Instance()->IsReconnect()) {
            // initiate autoreconnect
            startReconnectTimer();
            WndManager::Instance()->HandleConnecting();
        } else {
            //WndManager::Instance()->HandleDisconnected();
            setState(ovsDisconnected);
        }
    } else if (word.compare("RECONNECTING", Qt::CaseInsensitive) == 0) {
        setState(ovsConnecting);
        WndManager::Instance()->HandleState(word);
    } else if (word.compare("AUTH", Qt::CaseInsensitive) == 0) {
        setState(ovsConnecting);
        WndManager::Instance()->HandleState(word);
    } else if (word.compare("GET_CONFIG", Qt::CaseInsensitive) == 0) {
        WndManager::Instance()->HandleState(word);
    } else if (word.compare("ASSIGN_IP", Qt::CaseInsensitive) == 0) {
        setState(ovsConnecting);
        WndManager::Instance()->HandleState(word);
    } else if (word.compare("TCP_CONNECT'", Qt::CaseInsensitive) == 0) {
        WndManager::Instance()->HandleState(word);
    } else if (word.compare("RESOLVE", Qt::CaseInsensitive) == 0) {
        WndManager::Instance()->HandleState(word);
        if (OsSpecific::Instance()->IsNetdown()) {
            stop();
            WndManager::Instance()->ErrMsg("Turn Internet connection on manually, please");
        }
    } else {
        log::logt("Got unknown state word " + word);
        WndManager::Instance()->HandleState(word);
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
        log::logt(s);
        log::logt("processing plain word '" + word + "'");
        if (word.compare("SUCCESS", Qt::CaseInsensitive) == 0) {
            ;
        } else {
            if (word.compare("ERROR", Qt::CaseInsensitive) == 0 || word.compare("FATAL", Qt::CaseInsensitive) == 0) {
                disconnectFromOpenvpnSocket();
                LoginWindow::Instance()->BlockOnDisconnect();
                WndManager::Instance()->ErrMsg(s);
            }
        }
    }
}

void OpenvpnManager::parseSocketQueryWord(const QString & word, const QString & s)
{
    // TODO: -1 hash_map
    // TODO: -2 state machine

    log::logt("ProcessRtWord(): '"  + s + "'");
    log::logt("processing RT word '" + word + "'");

    if (word.compare("INFO", Qt::CaseInsensitive) == 0) {
        // INFO:
        // just ignore
    } else if (word.compare("HOLD", Qt::CaseInsensitive) == 0) {
        if (s.indexOf("hold release") > -1) {
            log::logt("hold off");
            mSocket->write("hold off\n");
            log::logt("hold release");
            mSocket->write("hold release\n");
            mSocket->flush();
        }
    } else if (word.compare("PASSWORD", Qt::CaseInsensitive) == 0) {
        if (s.indexOf("Need 'Auth' username/password") > -1) {
            log::logt("sending vpn login+password");
            QString u1 = "username \"Auth\" \"" + EscapePsw(AuthManager::Instance()->VpnName()) + "\"\n";
            QString p1 = "password \"Auth\" \"" + EscapePsw(AuthManager::Instance()->VpnPsw()) + "\"\n";
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

    setState(ovsDisconnected);
    WndManager::Instance()->ErrMsg(msg);
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
        ;        // TODO: -0
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

        QTemporaryFile file(QDir::tempPath() + "/safejumper-tmp-XXXXXX.sh");
        QTemporaryFile outf(QDir::tempPath() + "/safejumper-tmp-XXXXXX.out");
        if (file.open())
            if (outf.open()) {
                QString script = QString(OsSpecific::Instance()->isOpenvpnRunningCommand()) + " > " + outf.fileName();
                file.write(script.toLatin1());
                file.flush();

                int re = QProcess::execute("/bin/bash", QStringList() << file.fileName());
                switch (re) {
                case -2:
                    log::logt("openvpnRunning(): -2 the process cannot be started");
                    break;
                case -1:
                    log::logt("openvpnRunning(): -1 the process crashed");
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
                    log::logt("IsOvRunning(): ps-grep return code = " + QString::number(re));
                    break;
                }
            }
#endif  // else WIN32
    }
//log::logt(QString("IsOvRunning() returns ") + QString(is ? "true": "false") );
    return running;
}

void OpenvpnManager::killRunningOpenvpn()
{
    log::logt(QString("KillRunningOV() enter"));
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
                    OsSpecific::Instance()->ExecAsRoot("/bin/kill", a);
                } catch(std::exception & ex) {
                    log::logt(ex.what());
                    WndManager::Instance()->ErrMsg(ex.what());
                }
            }
        }
    }
    log::logt(QString("KillRunningOV() exit"));
}

void OpenvpnManager::startPortLoop(bool port)
{
    if (state() != ovsConnected) {      // if not connected already during this call
        mInPortLoop = true;
//      uint dt = QDateTime::currentDateTimeUtc().toTime_t();
//      if ((dt - _dtStart) > G_Delay_PortQuestion)
        {
            mChangingPorts = port;
            tryNextPort();
        }
    }
}

void OpenvpnManager::socketConnected()
{
    log::logt("Socket to openvpn connected");
}

void OpenvpnManager::tryNextPort()
{
    mStartDateTime = QDateTime::currentDateTimeUtc().toTime_t();      // force start interval - prevent double port change
    if (mChangingPorts)
        Setting::Instance()->SwitchToNextPort();
    else
        Setting::Instance()->SwitchToNextNode();
    launchOpenvpn();
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
            launchOpenvpn();    // force stop then start
        }
    } else {
        launchOpenvpn();
    }
}

