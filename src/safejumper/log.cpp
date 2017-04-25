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

#include "log.h"


#include <QFile>
#include <QDateTime>

#include "scr_logs.h"
#include "pathhelper.h"
#include "setting.h"

void log::logt(const QString & s)
{
    // Don't log anything if logging is disabled
    if (!Setting::instance()->logging())
        return;

    QDateTime now = QDateTime::currentDateTimeUtc();
    QString s1 = now.toString("yyyy-MM-dd-HH-mm-ss ") + s;
    s1 +=  + "\n";
    QFile ff(PathHelper::Instance()->safejumperLogFilename());
    if (ff.open(QIODevice::Append)) {
        ff.write(s1.toLatin1());
        ff.flush();
        ff.close();
    }

    if (Scr_Logs::IsExists())
        Scr_Logs::Instance()->Log(s1);
}
