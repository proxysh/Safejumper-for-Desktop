#include "acconnectto.h"
#include "openvpnmanager.h"

AcConnectto::AcConnectto(size_t srvid, QObject *parent)
    : QObject(parent)
    , _srvid(srvid)
{}

AcConnectto::~AcConnectto()
{}

void AcConnectto::ac_ConnectTo()
{
    OpenvpnManager::Instance()->startWithServer(_srvid);
}
