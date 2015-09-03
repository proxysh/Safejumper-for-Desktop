#include "acconnectto.h"
#include "ctr_openvpn.h"

AcConnectto::AcConnectto(size_t srvid, QObject *parent)
	: QObject(parent)
	, _srvid(srvid)
{}

AcConnectto::~AcConnectto()
{}

void AcConnectto::ac_ConnectTo()
{
	Ctr_Openvpn::Instance()->Start(_srvid);
}
