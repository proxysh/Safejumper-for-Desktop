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

#ifndef OSSPECIFIC_H
#define OSSPECIFIC_H

#include <memory>
#include <stdint.h>

#include <QString>
#include <QProcess>

class OsSpecific: public QObject
{
    Q_OBJECT
public:
    static bool exists();
    static OsSpecific *instance();
    static void cleanup();
    ~OsSpecific();

    bool hasInsecureWifi();
    void fixDnsLeak();

    QString runCommandFast(const char * cmd, uint16_t ms = 500) const;			// returns stdout
    QString runCommandFast(const QString & cmd, uint16_t ms = 500) const;
    static QString runCommandFast(const QString &cmd, const QStringList &arguments);

private:
    OsSpecific();
    static std::auto_ptr<OsSpecific> mInstance;
};

#endif // OSSPECIFIC_H
