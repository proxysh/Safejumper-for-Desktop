#include "openvpnmanager.h"

#include <QFile>
#include <QDir>
#include <stdexcept>

#include "authmanager.h"
#include "osspecific.h"
#include "scr_connect.h"
#include "scr_map.h"
#include "wndmanager.h"
#include "setting.h"
#include "log.h"
#include "sjmainwindow.h"
#include "pathhelper.h"

#ifdef MONITOR_TOOL
#include "scr_table.h"
#endif  // MONITOR_TOOL

OpenvpnManager::OpenvpnManager()
    : _state(ovsDisconnected),
      _pid(0),
      mSocket(0),
      mStateTimer(NULL)
{}

OpenvpnManager::~OpenvpnManager()
{

}

void OpenvpnManager::cleanup()
{
    if (_inst.get() != NULL)
        delete _inst.release();
}

bool OpenvpnManager::exists()
{
    return (_inst.get() != NULL);
}

OpenvpnManager::OvState OpenvpnManager::state()
{
    return _state;
}

std::auto_ptr<OpenvpnManager> OpenvpnManager::_inst;
OpenvpnManager * OpenvpnManager::Instance()
{
    if (!_inst.get())
        _inst.reset(new OpenvpnManager());
    return _inst.get();
}

void OpenvpnManager::startWithServer(size_t srv)
{
    Scr_Map::Instance()->SetServer(srv);
    start();
}

void OpenvpnManager::start()
{
    WndManager::Instance()->ClosePortDlg();
    _PortDlgShown = false;
    _InPortLoop = false;    // TODO: -0
    _reconnect_attempt = 0;
#ifdef MONITOR_TOOL
    _StopLoop = false;
    _attempt = 0;
    _InNextPort = false;
    Setting::Instance()->InitLoop();
#endif  // MONITOR_TOOL

    launchOpenvpn();
    log::logt("out Ctr_Openvpn::Start()");
#ifdef MONITOR_TOOL
    WndManager::Instance()->ShowTable();
#endif
}

void OpenvpnManager::launchOpenvpn()
{
#ifdef MONITOR_TOOL
    if (_StopLoop)
        return;
#endif  // MONITOR_TOOL
    {
        stop();
        // TODO: -1 cleanup
    }

    if (AuthManager::Instance()->IsLoggedin()) {
        setState(ovsConnecting);
        int enc = Setting::Encryption();
        bool obfs = enc == ENCRYPTION_OBFS_TOR;
        try {
            if (obfs) {
                OsSpecific::Instance()->RunObfs(Setting::Instance()->Server(), Setting::Instance()->Port(), "1050");
                if (!OsSpecific::Instance()->IsObfsRunning())
                    throw std::runtime_error("Cannot run Obfs proxy");
            }
        } catch(std::exception & ex) {
            log::logt(ex.what());
            WndManager::Instance()->ErrMsg(QString(ex.what()));
            return;
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

        _LocalAddr = "127.0.0.1";
        _LocalPort = Setting::Instance()->LocalPort().toInt();

//      AuthManager::Instance()->SetNewIp("");
        _tunerr = false;
        _err = false;
#ifdef MONITOR_TOOL
        _err_msg = "";
#endif// MONITOR_TOOL
        _processing = false;
        if (NULL != _watcher.get()) {       // OpenVPN log file watcher
            _watcher->removePath(PathHelper::Instance()->OpenvpnLogPfn());
            delete _watcher.release();
        }

        disconnectFromOpenvpnSocket();
#ifndef NO_PARAMFILE
        QFile ff(PathHelper::Instance()->OpenvpnConfigPfn());
        if (!ff.open(QIODevice::WriteOnly)) {
            QString se = "Cannot write config file '" + PathHelper::Instance()->OpenvpnConfigPfn() + "'";
            log::logt(se);
            WndManager::Instance()->ErrMsg(se);
            return;
        }
        ff.write("client\n");
        ff.write("dev tun\n");
        ff.write("proto ");
        ff.write(Setting::Instance()->Protocol().toLatin1());
        ff.write("\n");       // "tcp"/"udp"
//ff.write("proto udp\n");
//ff.write("proto tcp\n");

        ff.write("remote ");
        ff.write(Setting::Instance()->Server().toLatin1());
        ff.write(" ");
        ff.write(Setting::Instance()->Port().toLatin1());
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
        ff.write("route-delay 2\n");

        if (enc == ENCRYPTION_ECC || enc == ENCRYPTION_ECCXOR) {
//          ff.write("tls-cipher ECDHE-ECDSA-AES256-GCM-SHA384\n");
            ff.write("tls-cipher TLS-ECDHE-ECDSA-WITH-AES-256-GCM-SHA384\n");

            //ff.write("tls-cipher ECDH\n");
            //ff.write("tls-cipher !ECDH\n");

            ff.write("ecdh-curve secp384r1\n");

            if (ENCRYPTION_ECCXOR == enc)
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
            ff.write("connect-retry-max 1\n");
            ff.write("socks-proxy 127.0.0.1 1050\n");
            ff.write("route ");
            ff.write(Setting::Instance()->Server().toLatin1());
            ff.write(" 255.255.255.255 net_gateway\n");
        }

        ff.flush();
        ff.close();
#endif  // NO_PARAMFILE

        QStringList args;
        args
//          << "--auth-nocache"
#ifndef NO_PARAMFILE
                << "--config" << PathHelper::Instance()->OpenvpnConfigPfn() // /tmp/proxysh.ovpn
#endif
#ifdef NO_PARAMFILE
                << "--client"
#endif

#ifndef Q_OS_WIN
                << "--daemon" // does not work at windows
#endif

#ifdef NO_PARAMFILE
                << "--dev tun0"
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
                << "--allow-pull-fqdn"
#endif
                << "--management" << _LocalAddr << Setting::Instance()->LocalPort()
                << "--management-hold"
                << "--management-query-passwords"
#ifdef MONITOR_TOOL
                << "--log-append" << PathHelper::Instance()->OpenvpnLogPfn()            // /tmp/openvpn.log
#else
                << "--log" << PathHelper::Instance()->OpenvpnLogPfn()           // /tmp/openvpn.log
#endif  // MONITOR_TOOL

//          << "--script-security" << "3" << "system"
//          << "--script-security" << "2" << "execve"       // https://openvpn.net/index.php/open-source/documentation/manuals/69-openvpn-21.html
                << "--script-security" << "3"

#ifndef Q_OS_WIN            // TODO: -0 DNS on linux
                << "--up" << PathHelper::Instance()->UpScriptPfn()              // /Applications/Safejumper.app/Contents/Resources/client.up.safejumper.sh
                << "--down" << PathHelper::Instance()->DownScriptPfn()      // /Applications/Safejumper.app/Contents/Resources/client.down.safejumper.sh
#endif
                << "--up-restart"
                ;

        // TODO: -1 download cert from proxy.sh
        if (enc != ENCRYPTION_ECC && enc != ENCRYPTION_ECCXOR)
            args << "--ca" << PathHelper::Instance()->ProxyshCaCert();    // /tmp/proxysh.crt

        if (Setting::Instance()->IsFixDns() || !Setting::Instance()->Dns1().isEmpty() || !Setting::Instance()->Dns2().isEmpty())
            OsSpecific::Instance()->FixDnsLeak();

        if (!Setting::Instance()->Dns1().isEmpty())
            args << "--dhcp-option" << "DNS" << Setting::Instance()->Dns1();
        if (!Setting::Instance()->Dns2().isEmpty())
            args << "--dhcp-option" << "DNS" << Setting::Instance()->Dns2();

        QString prog = PathHelper::Instance()->OpenvpnPathfilename();
        log::logt("Prog is: " + prog);
        QString params = args.join(' ');
        log::logt("Args are:" + params);

        bool ok = false;
        try {
#ifndef Q_OS_WIN
            _paramFile.reset(new QTemporaryFile());
            if (!_paramFile->open())
                throw std::runtime_error("Cannot create tmp file.");

            OsSpecific::Instance()->SetRights();        // lean inside, throw on error

            _paramFile->write(params.toLatin1());
            _paramFile->flush();

            QStringList arg3;
            arg3 << _paramFile->fileName();
            _paramFile->close();

//#ifdef Q_OS_MAC
//          log::logt("######  touch launcher ####");
//          OsSpecific::Instance()->RunFastCmd("touch -a " + PathHelper::Instance()->LauncherPfn());
//#endif
            log::logt("######  before exec ####");

#ifdef Q_OS_LINUX
        OsSpecific::Instance()->ExecAsRoot(PathHelper::Instance()->LauncherPfn(), arg3);
#else
//#ifdef Q_OS_MAC
//          OsSpecific::Instance()->ExecAsRoot(prog, args);     // force password dialog; without launcher
//#else
            int r3 = QProcess::execute(PathHelper::Instance()->LauncherPfn(), arg3);    // 30ms block internally
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
#endif
//#endif

            log::logt("before attaching to OpenVPN");
            connectToOpenvpnSocket();   // TODO: -1 wait for slow starting cases
            setupFileWatcher();
            log::logt("after attaching to OpenVPN");
#else
            Scr_Connect * sc = Scr_Connect::Instance();
            _process.reset(new QProcess());
            sc->connect(_process.get(), SIGNAL(error(QProcess::ProcessError)), sc, SLOT(ConnectError(QProcess::ProcessError)));
            sc->connect(_process.get(), SIGNAL(started()), sc, SLOT(ConnectStarted()));
            sc->connect(_process.get(), SIGNAL(stateChanged(QProcess::ProcessState)), sc, SLOT(ConnectStateChanged(QProcess::ProcessState)));
            sc->connect(_process.get(), SIGNAL(finished(int, QProcess::ExitStatus)), sc, SLOT(ConnectFinished(int, QProcess::ExitStatus)));
            sc->connect(_process.get(), SIGNAL(readyReadStandardError()), sc, SLOT(ConnectStderr()));
            sc->connect(_process.get(), SIGNAL(readyReadStandardOutput()), sc, SLOT(ConnectStdout()));
            _process->start(prog, args);
            _process->waitForStarted(2000);
            log::logt("Process ID is: " + QString::number(_process->processId()));
#endif
            ok = true;
        } catch(std::exception & ex) {
            log::logt(ex.what());
            WndManager::Instance()->ErrMsg(ex.what());
        }

        if (_paramFile.get())
            delete _paramFile.release();

        if (ok) {
            _dtStart = QDateTime::currentDateTimeUtc().toTime_t();
            startTimer();
        } else
            setState(ovsDisconnected);

    }
    log::logt("exiting Ctr_Openvpn::StartImpl()");
#undef NO_PARAMFILE
}

#ifdef MONITOR_TOOL
void Ctr_Openvpn::ReconnectIfMax()
{
    ++_attempt;
    if (_attempt > G_Max_Reconnect) {
        _attempt = 0;
        Scr_Table::Instance()->SetStatus(Setting::Encryption(), Setting::Instance()->ServerID(), Setting::Instance()->CurrProto(), snsTimeout);
        ToNextPort();           // calls StartImpl inside
    } else {
        Scr_Table::Instance()->SetStatus(Setting::Encryption(), Setting::Instance()->ServerID(), Setting::Instance()->CurrProto(), "attempt "+ QString::number(_attempt));
        StartImpl();        // restart
    }
}
#endif  // MONITOR_TOOL

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
    } else {
        if (state() == ovsConnected) {
            // handle crush
            if (Setting::Instance()->IsReconnect())
                start();
            else
                setState(ovsDisconnected);
        }
    }

    if (state() == ovsConnecting) {
        uint d = QDateTime::currentDateTimeUtc().toTime_t() - _dtStart;
#ifdef MONITOR_TOOL
        if (d > G_Delay_OneCheck)
            ReconnectIfMax();
#else
        if (!_PortDlgShown && !_InPortLoop) {
            if (d > G_Delay_PortQuestion) {
                _PortDlgShown = true;
                WndManager::Instance()->ShowPortDlg();
            }
        }

        if (_InPortLoop) {
            if (d > G_Delay_PortIteration)
                tryNextPort();
        }
#endif  // MONITOR_TOOL
    }
}

void OpenvpnManager::openvpnLogfileChanged(const QString & pfn)
{
    if (_processing || _err)
        return;
    _processing = true;
    if (pfn == PathHelper::Instance()->OpenvpnLogPfn()) {
        QFile f(pfn);
        QByteArray ba;
        if (f.open(QIODevice::ReadOnly)) {      // TODO: -2 ensure non-blocking
            if (_lastpos > f.size())        // file was truncated
                _lastpos = 0;
            if ((_lastpos + 1) < f.size()) {
                f.seek(_lastpos);
                ba = f.read(f.size() - _lastpos);
                _lastpos = f.size();
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
    _processing = false;
}

void OpenvpnManager::parseOpenvpnLogLine(const QString & s)
{
    // TODO: -2 state machine
    if (s.contains("MANAGEMENT: CMD 'state'", Qt::CaseInsensitive)) {
        ;   // skip our commands
    } else {
        log::logt("OpenVPNlogfile: " + s);
        if (s.contains("TCPv4_CLIENT link remote:", Qt::CaseInsensitive)) {
            parseNewIp(s);
        } else {
            if (s.contains("Initialization Sequence Completed:", Qt::CaseInsensitive)) {
                gotConnected(s);
            } else {
                if (s.contains("Opening utun (connect(AF_SYS_CONTROL)): Operation not permitted", Qt::CaseInsensitive)) {
                    gotTunErr(s);
                }
            }
        }
    }
}

void OpenvpnManager::setState(OvState st)
{
    if (st != _state) {
        _state = st;
        log::logt("Set state " + QString::number(st));
        switch (st) {
        case ovsConnected: {
            WndManager::Instance()->HandleConnected();
#ifndef MONITOR_TOOL
            OsSpecific::Instance()->SetNetdown(false);
#endif  // MONITOR_TOOL

#ifdef MONITOR_TOOL
            // TODO: -2 Verify connection via http
            ToNextPort();
#endif  // MONITOR_TOOL
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
#ifdef MONITOR_TOOL
            ToNextPort();
#endif  // MONITOR_TOOL
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
            AuthManager::Instance()->SetNewIp(ip);
    }

    AuthManager::Instance()->ForwardPorts();
}

void OpenvpnManager::gotTunErr(const QString & s)
{
    if (!_tunerr && !_err) {
        _tunerr = true;
        _err = true;
#ifdef MONITOR_TOOL
        _err_msg = "err TUN";
#endif  // MONITOR_TOOL
        cancel(s);
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
            AuthManager::Instance()->SetNewIp(ip);
            Scr_Connect::Instance()->UpdNewIp(ip);
        }
    }
}

void OpenvpnManager::cancel(const QString & msg)
{
    stop();
    SjMainWindow::Instance()->BlockOnDisconnect();
    WndManager::Instance()->ErrMsg(msg);
}

/*
void Ctr_Openvpn::Cancel()
{
    if (IsOvRunning())
        Stop();
    SetState(ovsDisconnected);
}
*/
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
            QTcpSocket * to = mSocket.release();
            to->deleteLater();

            for (int cn = 0; cn < 8 && openvpnRunning(); ++cn)
                QThread::msleep(100);       // just sleep now; without this delay it fails to jump
        }

        QThread::msleep(200);
        removeProcess();

        if (openvpnRunning())
            log::logt("Stop(): cannot soft stop OpenVPN process");
    }
    if (Setting::Encryption() == ENCRYPTION_OBFS_TOR)
        if (OsSpecific::Instance()->IsObfsRunning()) {
            // TODO: -0 stop
        }

    setState(ovsDisconnected);
}

#ifdef MONITOR_TOOL
void Ctr_Openvpn::StopLoop()
{
    _StopLoop = true;
    Stop();
}
#endif  // MONITOR_TOOL

void OpenvpnManager::removeProcess()
{
    if (_process.get() != NULL) {
        QProcess * t = _process.release();
        t->deleteLater();
    }
}

void OpenvpnManager::logStderr()
{
    log::logt("ReadStderr(): " + _process->readAllStandardError());
}

void OpenvpnManager::logStdout()
{
    log::logt("ReadStdout(): " + _process->readAllStandardOutput());
}

void OpenvpnManager::processStateChanged(QProcess::ProcessState st)
{
    // TODO: handle open vpn  process startup
    setupFileWatcher();
}

void OpenvpnManager::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // OpenVpn crushed or just spawn a child and exit during startup
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
    if (_watcher.get() == NULL) {
        QFile f(PathHelper::Instance()->OpenvpnLogPfn());
        if (f.exists()) {
            _lastpos = 0;       // OpenVpn will truncate

            _watcher.reset(new QFileSystemWatcher());
            _watcher->addPath(PathHelper::Instance()->OpenvpnLogPfn());

            log::logt("Monitoring " + PathHelper::Instance()->OpenvpnLogPfn());
            connect(_watcher.get(), SIGNAL(fileChanged(const QString &)),
                    this, SLOT(openvpnLogfileChanged(const QString &)));
        }
    }
}

void OpenvpnManager::disconnectFromOpenvpnSocket()
{
    log::logt("disconnecting from openvpn management socket");


    if (mSocket.get() != NULL) {
        disconnect(mSocket.get(), SIGNAL(error(QAbstractSocket::SocketError)),
                   this, SLOT(socketError(QAbstractSocket::SocketError)));
        disconnect(mSocket.get(), SIGNAL(readyRead()),
                   this, SLOT(socketReadyRead()));

        mSocket->abort();
        mSocket.release()->deleteLater();
    }
    log::logt("done disconnecting from openvpn management socket");
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
    mSocket->connectToHost(_LocalAddr, _LocalPort);
    log::logt("done connecting to openvpn management socket");
}

void OpenvpnManager::socketError(QAbstractSocket::SocketError error)
{
    log::logt("Error connecting to OpenVPN management socket" + QString::number(error));
    if (NULL != mSocket.get())
        mSocket.release()->deleteLater();
}

void OpenvpnManager::socketReadyRead()
{
//  log::logt("Soc_ReadyRead()");
    if (!mSocket->canReadLine())
        return;
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
    if (word != _prev_st_word) {
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
#ifdef MONITOR_TOOL
        ReconnectIfMax();
#endif  // MONITOR_TOOL
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
        WndManager::Instance()->HandleState(word);
    }
    _prev_st_word = word;
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
                SjMainWindow::Instance()->BlockOnDisconnect();
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
                _err = true;
#ifdef MONITOR_TOOL
                _err_msg = "err auth";
#endif// MONITOR_TOOL
                // OpenVpn exiting
                showErrorMessageCleanup(s.mid(p + 1));
            }
        }
    } else if (word.compare("SUCCES", Qt::CaseInsensitive) == 0) {
        ;
    } else {
        if (word.compare("FATAL", Qt::CaseInsensitive) == 0) {
            _err = true;
#ifdef MONITOR_TOOL
            _err_msg = "err fatal";
#endif  // MONITOR_TOOL
            int p = s.indexOf(':');
            QString msg = s.mid(p + 1);
            cancel(msg);
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
    bool is = false;

    if (NULL != _process.get()) {
        if (QProcess::Running == _process->state() || QProcess::Starting == _process->state()) {
            is = true;
        } else {
            //is = true;        // HACK: -0 locate OpenVpn child process
        }
    }

    if (!is)
        if (NULL != mSocket.get()) {
            if (mSocket->isOpen()) {
                is = true;
            }
        }

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
                QString script = QString(OsSpecific::Instance()->IsRunningCmd()) + " > " + outf.fileName();
                file.write(script.toLatin1());
                file.flush();


                int re = QProcess::execute("/bin/bash", QStringList() << file.fileName());
                switch (re) {
                case -2:
                    log::logt("IsOvRunning(): -2 the process cannot be started");
                    break;
                case -1:
                    log::logt("IsOvRunning(): -1 the process crashes");
                    break;
                case 0: {
                    QByteArray ba = outf.readAll();
                    QString s2(ba);
                    QString s3 = s2.trimmed();
                    int p = s3.indexOf(' ');
                    QString s4 = s3.mid(0, p);
                    bool converted;
                    _pid = s4.toInt(&converted);
                    if (converted) {
                        if (_pid > 0) {
                            is = true;
                            //                              AttachMgmt();
                        }
                    }
                    break;
                }
                case 1:
                    is = false;     // no lines
                    break;
                case 2:
                    is = false;     // grep failure
                    break;
                default:
                    log::logt("IsOvRunning(): ps-grep return code = " + QString::number(re));
                    break;
                }
            }
#endif  // else WIN32
    }
//log::logt(QString("IsOvRunning() returns ") + QString(is ? "true": "false") );
    return is;
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

            if (_pid > 0) {
                QStringList a;
                a << "-9" << QString::number(_pid);
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
        _InPortLoop = true;
//      uint dt = QDateTime::currentDateTimeUtc().toTime_t();
//      if ((dt - _dtStart) > G_Delay_PortQuestion)
        {
            _IsPort = port;
            tryNextPort();
        }
    }
}

void OpenvpnManager::tryNextPort()
{
    _dtStart = QDateTime::currentDateTimeUtc().toTime_t();      // force start interval - prevent double port change
#ifdef MONITOR_TOOL
    if (!_StopLoop && !_InNextPort) {
        _InNextPort = true;
        bool InLoop = Setting::Instance()->SwitchToNext();
        if (InLoop && !_StopLoop)
            StartImpl();
        else
            Stop();     // will reiterate here
        _InNextPort = false;
    }
#else
    if (_IsPort)
        Setting::Instance()->SwitchToNextPort();
    else
        Setting::Instance()->SwitchToNextNode();
    launchOpenvpn();
#endif  // MONITOR_TOOL
}

void OpenvpnManager::startReconnectTimer()
{
    QTimer::singleShot(1000, this, SLOT(reconnectTimeout()));
}

void OpenvpnManager::reconnectTimeout()
{
    ++_reconnect_attempt;
    if (openvpnRunning()) {
        if (_reconnect_attempt < G_Max_Reconnect)
            QTimer::singleShot(200, this, SLOT(reconnectTimeout()));
        else {
            stop();
            launchOpenvpn();    // force stop then start
        }
    } else {
        launchOpenvpn();
    }
}

