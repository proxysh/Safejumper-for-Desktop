/***************************************************************************
 *   Copyright (C) 2018 by Jeremy Whiting <jeremypwhiting@gmail.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation version 2 of the License.                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "server.h"

#include "setting.h"

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
    if (favorite)
        Setting::instance()->addFavorite(mAddress);
    else
        Setting::instance()->removeFavorite(mAddress);
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

void AServer::setId(int id)
{
    mId = id;
}

int AServer::id() const
{
    return mId;
}
