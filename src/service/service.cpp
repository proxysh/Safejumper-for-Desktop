#include "common.h"
#include "service.h"
#include "servicelog.h"

#include <QThread>

Service::Service(int argc, char **argv)
    : QtService<QCoreApplication>(argc, argv, kAppName)
    , m_VPNServer(nullptr)
{
//    Log::serviceLog("*** Service constructed ***");
//    QThread::sleep(30);
    setServiceDescription(QString("%1 VPN service.").arg(kAppName));
    setStartupType(QtServiceController::AutoStartup);
}

Service::~Service()
{
//    Log::serviceLog("*** service destructed ***");
    if (m_VPNServer) {
        m_VPNServer->stop();
        m_VPNServer->deleteLater();
        m_VPNServer = nullptr;
    }
}

void Service::start()
{
    Log::serviceLog("*** service start called ***");
    m_VPNServer = new QVPNServer(this);
    Log::serviceLog("*** QVPNServer created, now starting it");
    m_VPNServer->start();
    Log::serviceLog("*** QVPNServer started");
}

void Service::stop()
{
    Log::serviceLog("*** service stop called ***");
    m_VPNServer->stop();
    m_VPNServer->deleteLater();
    m_VPNServer = nullptr;
}

void Service::processCommand(int code)
{
    Q_UNUSED(code);
}
