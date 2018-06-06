#ifndef QVPNSERVER_H
#define QVPNSERVER_H

#include "qvpnclientconnection.h"
#include <QLocalServer>
#include <QLocalSocket>
#include <QProcess>
#include <QObject>
#include <QList>


class QVPNServer : public QLocalServer
{
    Q_OBJECT
public:
    explicit QVPNServer(QObject *parent = 0);
    virtual ~QVPNServer();

    void start();
    void stop();
    static QVPNServer *instance();

signals:

protected:
    void incomingConnection(quintptr socketDescriptor) override;

private slots:
    void worker_destroyed(QObject *obj);

private:
    QList<QVPNClientConnection *> m_workers;
};

#endif // QVPNSERVER_H
