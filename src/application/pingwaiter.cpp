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

#include "pingwaiter.h"
#include "authmanager.h"

PingWaiter::PingWaiter(size_t idWaiter, QObject *parent)
    : QObject(parent)
    , _idWaiter(idWaiter)
{}

PingWaiter::~PingWaiter()
{}

void PingWaiter::PingFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    AuthManager::instance()->pingComplete(_idWaiter);
}

void PingWaiter::PingError(QProcess::ProcessError)
{
    AuthManager::instance()->pingError(_idWaiter);
}

void PingWaiter::Timer_Terminate()
{
    AuthManager::instance()->pingTerminated(_idWaiter);
}


