#include "qvpnserver.h"
#include "qvpnclientconnection.h"
//#include "qvpnsettings.h"
#include "service.h"
#include "common.h"
//#include "qapplog.h"
#include <QLocalServer>
#include <QLocalSocket>
#include <QSettings>
#include <QHash>


#include "servicelog.h"
#ifdef Q_OS_WIN
#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#endif


static QVPNServer *m_instance = nullptr;

QVPNServer::QVPNServer(QObject *parent)
    : QLocalServer(parent)
{
    Log::serviceLog("*** QVPNServer constructed");
//    QAppLog::Log(__FUNCTION__, QAppLog::QLogLevelInformation);

    m_instance = this;

    setSocketOptions(QLocalServer::WorldAccessOption);
}

QVPNServer::~QVPNServer()
{
    Log::serviceLog("*** QVPNServer destructed");
//    QAppLog::Log(__FUNCTION__, QAppLog::QLogLevelInformation);

    m_instance = nullptr;
}

void QVPNServer::start()
{
    Log::serviceLog("*** QVPNServer start called");
//    QAppLog::Log(__FUNCTION__, QAppLog::QLogLevelInformation);

    QSettings settings;

    switch (settings.value("LogLevel", 4).toInt()) {
    case 0:
//        QAppLog::setLevel(QAppLog::QLogLevelCritical, true);
        break;
    case 1:
//        QAppLog::setLevel(QAppLog::QLogLevelError, true);
        break;
    case 2:
//        QAppLog::setLevel(QAppLog::QLogLevelWarning, true);
        break;
    case 3:
//        QAppLog::setLevel(QAppLog::QLogLevelDebug, true);
        break;
    case 4:
//        QAppLog::setLevel(QAppLog::QLogLevelInformation, true);
        break;
    default:
//        QAppLog::setLevel(QAppLog::QLogLevelWarning, true);
        break;
    };

    if (listen(kSocketName) == false) {
        Log::serviceLog("*** SafejumperVPN failed to listen for connections(another instance is running maybe?).");

        return;
    }
}

void QVPNServer::stop()
{
    Log::serviceLog("*** QVPNServer stop called");
//    QAppLog::Log(__FUNCTION__, QAppLog::QLogLevelInformation);

    disconnect(this, SLOT(newConnection()));

    if (isListening()) {
//        QAppLog::Log("Closing localsocket.", QAppLog::QLogLevelInformation);

        close();
    }
}

QVPNServer *QVPNServer::instance()
{
//    QAppLog::Log(__FUNCTION__, QAppLog::QLogLevelInformation);

    return m_instance;
}

void QVPNServer::incomingConnection(quintptr socketDescriptor)
{
    Log::serviceLog("*** Incoming connection in vpn service");
//    QAppLog::Log(__FUNCTION__, QAppLog::QLogLevelInformation);

    QVPNClientConnection *newWorker = new QVPNClientConnection(this, socketDescriptor);

    connect(newWorker, SIGNAL(destroyed(QObject*)), SLOT(worker_destroyed(QObject*)));

    m_workers.append(newWorker);
}

void QVPNServer::worker_destroyed(QObject *obj)
{
    Log::serviceLog("*** QVPNServer worker_destroyed");
//    QAppLog::Log(__FUNCTION__, QAppLog::QLogLevelInformation);

    m_workers.removeAll((QVPNClientConnection *)obj);
}
