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

#ifndef THREAD_FORWARDPORTS_H
#define THREAD_FORWARDPORTS_H

#include <QThread>
#include <QNetworkAccessManager>

#include "common.h"

class Thread_ForwardPorts : public QThread
{
    Q_OBJECT
public:
    Thread_ForwardPorts(QNetworkAccessManager & nam, const UVec & ports, QObject *parent = 0)
        : QThread(parent), _nam(nam) , _ports(ports) {}
    void run() Q_DECL_OVERRIDE;

private:
    QNetworkAccessManager & _nam;
    UVec _ports;
};

#endif // THREAD_FORWARDPORTS_H
