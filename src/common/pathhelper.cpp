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

#include "pathhelper.h"

#include "common.h"

#include <QDir>
#include <QCoreApplication>
#include <QProcess>

extern QCoreApplication * g_pTheApp;

std::auto_ptr<PathHelper> PathHelper::_inst;
PathHelper * PathHelper::Instance()
{
    if (!_inst.get())
        _inst.reset(new PathHelper());
    return _inst.get();
}

bool PathHelper::exists()
{
    return (_inst.get() != nullptr);
}

void PathHelper::cleanup()
{
    if (_inst.get() != nullptr)
        delete _inst.release();
}

PathHelper::PathHelper()
{
    QDir dir(tempPath());
    if (!dir.exists())
        dir.mkpath(tempPath());
}

PathHelper::~PathHelper()
{}

QString PathHelper::resourcesPath()
{
#ifdef Q_OS_DARWIN
    QDir d(g_pTheApp->applicationDirPath());
    d.cdUp();
    d.cd("Resources");
    return d.canonicalPath();
#else
    return QCoreApplication::applicationDirPath();
#endif
}

QString PathHelper::tempPath()
{
    return QDir::homePath() + QString("/.%1/").arg(kLowerAppName);
}

QString PathHelper::serviceLogFilename()
{
#ifdef Q_OS_WIN
    return resourcesPath() + QString("/%1-service.log").arg(kLowerAppName);
#else
    return QString("/tmp/%1-service.log").arg(kLowerAppName);
#endif
}

QString PathHelper::applicationLogFilename()
{
    return tempPath() + QString("%1-debug.log").arg(kLowerAppName);
}
