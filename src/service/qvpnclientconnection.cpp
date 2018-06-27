#include "qvpnclientconnection.h"
#include "qvpnserver.h"

#include "servicelog.h"
#include "openvpnmanager.h"
#include "common.h"
#include "osspecific.h"

#include <QHostInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtServiceBase>


QVPNClientConnection::QVPNClientConnection(QObject *parent, quintptr socketDescriptor)
    : QObject(parent),
      m_socket(nullptr),
      m_state(vpnStateDisconnected),
      mReconnecting(false),
      mKillSwitch(false),
      mDisconnectRequested(false)
{
//    Log::serviceLog(__FUNCTION__);
    Log::serviceLog(QString("ClientConnection created with socket descriptor %1").arg(socketDescriptor));

    m_socket = new QLocalSocket(this);
    m_socket->setSocketDescriptor(socketDescriptor);

    connect(m_socket, &QLocalSocket::readyRead, this, &QVPNClientConnection::socket_readyRead);
    connect(m_socket, &QLocalSocket::disconnected, this, &QObject::deleteLater);

    connect(OpenvpnManager::instance(), &OpenvpnManager::stateChanged,
            this, &QVPNClientConnection::vpnStateChanged);
    connect(OpenvpnManager::instance(), &OpenvpnManager::stateWordChanged,
            this, &QVPNClientConnection::vpnStateWord);
    connect(OpenvpnManager::instance(), &OpenvpnManager::sendError,
            this, &QVPNClientConnection::vpnError);
    connect(OpenvpnManager::instance(), &OpenvpnManager::gotNewIp,
            this, &QVPNClientConnection::gotNewIp);
    connect(OpenvpnManager::instance(), &OpenvpnManager::timedOut,
            this, &QVPNClientConnection::timedOut);

    if (!m_socket->open(QIODevice::ReadWrite)) {
        Log::serviceLog("Can't open connection to client app.");
    }
}

QVPNClientConnection::~QVPNClientConnection()
{
//    Log::serviceLog(__FUNCTION__);
}

bool QVPNClientConnection::isOpen()
{
    return m_socket->isOpen();
}

void QVPNClientConnection::socket_readyRead()
{
    Log::serviceLog(__FUNCTION__);

    QLocalSocket *socket = (QLocalSocket *)sender();

    QByteArray bytesRead = socket->readAll();
    mReceiveBuffer.append(bytesRead);

    while (mReceiveBuffer.contains('\n')) {
        QByteArray command = mReceiveBuffer.split('\n').at(0);
        mReceiveBuffer.remove(0, command.length() + 1);
        Log::serviceLog(QString("Command read from socket: %1").arg(QString(command)));

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(command, &error);

        QJsonObject jObj = doc.object();

        switch(jObj.value("cmd").toInt()) {
        case cmdKillRunningOpenvpn: {
            OpenvpnManager::instance()->killRunningOpenvpn();
        }
        break;
        case cmdGetStatus: {
            Log::serviceLog(QString("%1: Get Status Command: state (%2)").arg(__FUNCTION__).arg(m_state));
            vpnStateChanged(m_state);
        }
        break;

        case cmdSetCredentials: {
            // Get credentials from json object
            QString username = jObj.value("vpnusername").toString();
            QString password = jObj.value("vpnpassword").toString();
            OpenvpnManager::instance()->setVPNCredentials(username, password);
        }
        break;

        case cmdNetdown: {
            OpenvpnManager::instance()->netDown(true);
        }
        break;
        case cmdStart: {
            QRegExp newlines("[\n\r]");

            // Get parameters from json object and pass to openvpn manager
            mEncryption = jObj.value("encryption").toInt();
            mHostname = jObj.value("server").toString().remove(newlines);
            mPort = jObj.value("port").toString().remove(newlines);
            mLocalPort = jObj.value("localport").toString().remove(newlines);
            mTcpOrUdp = jObj.value("tcporudp").toString().remove(newlines);

            bool fixDNS = jObj.value("fixDNS").toBool();
            bool disableIPv6 = jObj.value("disableIPv6").toBool();
            QString dns1 = jObj.value("dns1").toString().remove(newlines);
            QString dns2 = jObj.value("dns2").toString().remove(newlines);

            mReconnecting = true;

            Log::serviceLog(QString("%1: Start VPN Command").arg(__FUNCTION__));

            OpenvpnManager::instance()->netDown(false); // Bring network interfaces back up if they are down
            // If we are connected, disconnect first
            Log::serviceLog("Got cmdStart command from socket, calling start on vpn manager");

            vpnState state = OpenvpnManager::instance()->state();
            // If we are connected, disconnect first
            if (state == vpnStateConnected) {
                OpenvpnManager::instance()->stop();
            }

            OpenvpnManager::instance()->start(mEncryption,
                                              mHostname,
                                              mPort,
                                              mLocalPort,
                                              mTcpOrUdp,
                                              disableIPv6,
                                              fixDNS,
                                              dns1,
                                              dns2);
            mReconnecting = false;
        }
        break;

        case cmdStop: {
            mDisconnectRequested = true;
            Log::serviceLog(QString("%1: Stop VPN Command").arg(__FUNCTION__));
            OpenvpnManager::instance()->stop();		// handle visuals insde
            mDisconnectRequested = false;
        }
        break;

        default:
            Log::serviceLog(QString("%1 - Unknown command recieved.").arg(__FUNCTION__));
            break;
        }
    }
}

void QVPNClientConnection::vpnStateChanged(vpnState state)
{
    // Only do killswitch if we were connected and we are not now and kill switch is enabled
//    if (mKillSwitch &&
//            state == vpnStateDisconnected &&
//            m_state == vpnStateConnected &&
//            !mReconnecting && !mDisconnectRequested) {
//        OsSpecific::instance()->netDown(true);
//        OpenvpnManager::instance()->stop();		// handle visuals insde
//        return;
//    }

    m_state = state;

//    if ((state == vpnReconnecting)&&(m_vpn->killSwitch() == true)) {
//        m_vpn->killProcesses();
//    } else if ((state == AbstractVPNManager::Connected)||(state == AbstractVPNManager::Disconnected)) {
//        if (!m_LoginInProgress)
//            m_vpn->enableFirewall(false);
//        // TODO - for tests!
//        /*
//        if (m_vpn->killSwitch() == true)
//        {
//            Log::serviceLog(QString("=======Setting the killSwitch Off for tests"));
//            m_vpn->killProcesses();
//        }
//        */
//    }

    vpnStatus(state);

    if (state == vpnStateConnected)
//        && m_vpn->dnsLeakProtection())
    {
//        Log::serviceLog("Received Connected Signal, now starting DNS Leak protection");
//        m_vpn->startDNSLeakProtection();
    }
}

void QVPNClientConnection::vpnStateWord(OpenVPNStateWord word)
{
    // Send state word notification to gui

    QJsonObject jObj;

    jObj["code"] = notifyStatusWord;
    jObj["state"] = word;

    sendMessage(jObj);
}

void QVPNClientConnection::vpnStatus(vpnState state)
{
    QJsonObject jObj;

    jObj["code"] = notifyStatusChange;
    jObj["state"] = state;

    sendMessage(jObj);
}

void QVPNClientConnection::vpnError(const QString &message)
{
    QJsonObject jObj;

    jObj["code"] = notifyError;
    jObj["message"] = message;

    sendMessage(jObj);
}

void QVPNClientConnection::gotNewIp(const QString &ip)
{
    QJsonObject jObj;

    jObj["code"] = notifyGotIP;
    jObj["ip"] = ip;

    sendMessage(jObj);
}

void QVPNClientConnection::timedOut()
{
    QJsonObject jObj;

    jObj["code"] = notifyTimeout;

    sendMessage(jObj);
}

void QVPNClientConnection::sendMessage(const QJsonObject &jObj)
{
    QJsonDocument doc(jObj);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    m_socket->write(strJson.toLatin1(), strJson.length());
    m_socket->write("\n");
}
