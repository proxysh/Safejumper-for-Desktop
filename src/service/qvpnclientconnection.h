#ifndef QVPNCLIENTCONNECTION_H
#define QVPNCLIENTCONNECTION_H

#include "common.h"
#include <QHostInfo>
#include <QLocalSocket>
#include <QObject>

class QVPNClientConnection : public QObject
{
    Q_OBJECT
public:
    explicit QVPNClientConnection(QObject *parent, quintptr socketDescriptor);
    virtual ~QVPNClientConnection();

    bool isOpen();

private slots:
    void socket_readyRead();
    void vpnStateChanged(vpnState state);
    void vpnStateWord(OpenVPNStateWord word);
    void vpnStatus(vpnState state);
    void vpnError(const QString &message);
    void gotNewIp(const QString &ip);
    void timedOut();

private:
    void sendMessage(const QJsonObject &jObj);

    QLocalSocket *m_socket;
    QByteArray mReceiveBuffer;

    vpnState m_state;

    int mEncryption;
    QString mHostname;
    QString mPort;
    QString mLocalPort;
    QString mTcpOrUdp;
    bool mDisableIPv6;
    bool mFixDNS;
    QString mDNS1;
    QString mDNS2;

    bool mKillSwitch;
    bool mDNSLeakProtection;

    bool mReconnecting; // Set to true when reconnecting (switching servers)
    // To prevent kill switch when reconnecting

    bool mDisconnectRequested;
};

#endif // QVPNCLIENTCONNECTION_H
