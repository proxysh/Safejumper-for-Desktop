
#include "server.h"

AServer::AServer()
{
    mLoad = 0;
    mPing = -1;
}

AServer::~AServer()
{

}

void AServer::setName(const QString &name)
{
    mName = name;
    emit nameChanged();
}

const QString AServer::name() const
{
    return mName;
}

void AServer::setAddress(const QString &address)
{
    mAddress = address;
    emit addressChanged();
}

const QString AServer::address() const
{
    return mAddress;
}

void AServer::setIP(const QString &ip)
{
    mIP = ip;
    emit ipChanged();
}

const QString AServer::ip() const
{
    return mIP;
}

void AServer::setISO(const QString &iso)
{
    mIsoCode = iso.toUpper();
    emit isoChanged();
}

const QString AServer::iso() const
{
    return mIsoCode;
}

void AServer::setPorts(const QVariantList &ports)
{
    mPorts = ports;
    emit portsChanged();
}

const QVariantList AServer::ports() const
{
    return mPorts;
}

void AServer::setXorPorts(const QVariantList &xorPorts)
{
    mXorPorts = xorPorts;
    emit xorPortsChanged();
}

const QVariantList AServer::xorPorts() const
{
    return mXorPorts;
}

void AServer::setLoad(int load)
{
    mLoad = load;
    emit loadChanged();
}

int AServer::load() const
{
    return mLoad;
}

void AServer::setFavorite(bool favorite)
{
    mFavorite = favorite;
    emit favoriteChanged();
}

bool AServer::favorite() const
{
    return mFavorite;
}

void AServer::setPing(int ping)
{
    mPing = ping;
    emit pingChanged();
}

int AServer::ping() const
{
    return mPing;
}
