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

#ifndef PORTFORWARDER_H
#define PORTFORWARDER_H

#include <memory>

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "common.h"

// will be queued in the main loop and just chain rquests for port forwarding
class PortForwarder : public QObject
{
    Q_OBJECT
public:
    explicit PortForwarder(const UVec & ports, QNetworkAccessManager & nam, const QString & login, const QString & psw, QObject *parent = 0);
    ~PortForwarder();

    void StartFirst();
signals:

public slots:
    void FinishedFirstPage();
    void Finished_SetPort();
private:
    UVec _ports;
    const QString & _login;
    const QString & _psw;
    QNetworkAccessManager & _nam;

    std::auto_ptr<QNetworkReply> _reply;
    IIMap _to_set;	// <port slot IX in the API (1-based), port>
    IIMap::iterator _it_curr;

    void SetPort();
};

#endif // PORTFORWARDER_H
