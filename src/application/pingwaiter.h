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

#ifndef PINGWAITER_H
#define PINGWAITER_H

#include <QObject>
#include <QProcess>

class PingWaiter : public QObject
{
    Q_OBJECT
public:
    explicit PingWaiter(size_t idWaiter, QObject *parent = 0);
    ~PingWaiter();

private:
    size_t _idWaiter;

signals:

public slots:
    void PingFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void PingError(QProcess::ProcessError);
    void Timer_Terminate();
};

#endif // PINGWAITER_H
