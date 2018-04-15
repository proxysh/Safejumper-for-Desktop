#ifndef SERVICE_H
#define SERVICE_H

#include <QtCore>
#include "qtservice.h"
#include "qvpnserver.h"


class Service  : public QObject, public QtService<QCoreApplication>
{
    Q_OBJECT
public:
    Service(int argc, char **argv);
    virtual ~Service();

protected:
    void start() Q_DECL_OVERRIDE;
    void stop() Q_DECL_OVERRIDE;
    void processCommand(int code) Q_DECL_OVERRIDE;

private:
    QVPNServer *m_VPNServer;
};

#endif // SERVICE_H
