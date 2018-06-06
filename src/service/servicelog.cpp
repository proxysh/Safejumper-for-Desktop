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

#include "servicelog.h"

#include <QFile>
#include <QDateTime>

#include <QtDebug>

#include "servicepathhelper.h"

Log *Log::mInstance = 0;
bool Log::mEnabled = false;

void Log::logt(const QString & s)
{
    qDebug() << "Log: " << s;

    // Don't Log anything if Logging is disabled
    if (!Log::mEnabled)
        return;

    QDateTime now = QDateTime::currentDateTimeUtc();
    QString s1 = now.toString("yyyy-MM-dd-HH-mm-ss ") + s;
    s1 +=  + "\n";
    QFile ff(ServicePathHelper::Instance()->safejumperLogFilename());
    if (ff.open(QIODevice::Append)) {
        ff.write(s1.toLatin1());
        ff.flush();
        ff.close();
    }

    emit instance()->logMessage(s1);
}

void Log::serviceLog(const QString &s)
{
    qDebug() << "Log: " << s;

    // Don't Log anything if Logging is disabled
    if (!Log::mEnabled)
        return;

    QDateTime now = QDateTime::currentDateTime();
    QString s1 = now.toString("yyyy-MM-dd-HH-mm-ss ") + s;
    s1 +=  + "\n";
    QFile ff(ServicePathHelper::Instance()->safejumperServiceLogFilename());
    if (ff.open(QIODevice::Append)) {
        ff.write(s1.toLatin1());
        ff.flush();
        ff.close();
    }
}

Log *Log::instance()
{
    if (!Log::mInstance)
        mInstance = new Log;
    return mInstance;
}

void Log::enableLogging(bool enabled)
{
    mEnabled = enabled;
}
