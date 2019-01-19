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

#ifndef SERVERSMODEL_H
#define SERVERSMODEL_H

#include <QAbstractListModel>

#include "common.h"

#include "server.h"

#include <QList>
#include <QJsonArray>

class ServersModel: public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        nameRole = Qt::UserRole + 1,
        isoRole,
        ipRole,
        hostnameRole,
        loadRole,
        favoriteRole,
        portsRole,
        xorPortsRole,
        pingRole
    };

    ServersModel(QObject *parent=nullptr);
    virtual ~ServersModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QHash<int, QByteArray> roleNames() const;

    void updateServers(const QJsonArray &servers);

    Q_INVOKABLE AServer *server(int index);
    int count();

    void setPing(int index, int ping);

    QList<int> serversForEncryption(int encryption);

    // Which server indexes of the model are favorites
    Q_INVOKABLE QList<int> favoriteServers();

private:
    QList<AServer*> mServers;
};

#endif
