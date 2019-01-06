/***************************************************************************
 *   Copyright (C) 2017 by Jeremy Whiting <jeremypwhiting@gmail.com>       *
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

#ifndef SERVER_H
#define SERVER_H

#include <QString>
#include <QVariantList>

class AServer: public QObject {
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString address READ address WRITE setAddress NOTIFY addressChanged)
    Q_PROPERTY(QString ip READ ip WRITE setIP NOTIFY ipChanged)
    Q_PROPERTY(QString iso READ iso WRITE setISO NOTIFY isoChanged)
    Q_PROPERTY(QVariantList ports READ ports WRITE setPorts NOTIFY portsChanged)
    Q_PROPERTY(QVariantList xorPorts READ xorPorts WRITE setXorPorts NOTIFY xorPortsChanged)
    Q_PROPERTY(int load READ load WRITE setLoad NOTIFY loadChanged)
    Q_PROPERTY(int ping READ ping WRITE setPing NOTIFY pingChanged)
    Q_PROPERTY(bool favorite READ favorite WRITE setFavorite NOTIFY favoriteChanged)
    Q_PROPERTY(int id READ id)

public:
    AServer(QObject *parent = nullptr);
    virtual ~AServer();

    void setName(const QString &name);
    const QString name() const;

    void setAddress(const QString &address);
    const QString address() const;

    void setIP(const QString &ip);
    const QString ip() const;

    void setISO(const QString &iso);
    const QString iso() const;

    void setPorts(const QVariantList &ports);
    const QVariantList ports() const;
    void setPortNames(const QVariantList &portNames);

    void setXorPorts(const QVariantList &xorPorts);
    const QVariantList xorPorts() const;
    void setXorPortNames(const QVariantList &xorPortNames);

    void setLoad(int load);
    int load() const;

    void setFavorite(bool favorite);
    bool favorite() const;

    void setPing(int ping);
    int ping() const;

    void setId(int id);
    int id() const;

    Q_INVOKABLE const QVariantList supportedPorts(int encryption);

signals:
    void nameChanged();
    void addressChanged();
    void ipChanged();
    void isoChanged();
    void portsChanged();
    void xorPortsChanged();
    void loadChanged();
    void favoriteChanged();
    void pingChanged();

private:
    QString mName;       // "Chile Hub" - Hub at the end indicates hub
    QString mAddress;    // DNs
    QString mIP;
    QString mIsoCode;
    QVariantList mPorts;
    QVariantList mXorPorts;
    QVariantList mPortNames;
    QVariantList mXorPortNames;
    int mLoad;       // double
    bool mFavorite;
    int mPing;
    int mId;
};

#endif // COMMON_H
