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

#ifndef LOG_H
#define LOG_H

#include <QObject>
#include <QString>

class Log: public QObject
{
    Q_OBJECT
public:
    static void logt(const QString & s);

    static void serviceLog(const QString &s);

    static Log* instance();

    void enableLogging(bool enabled);

signals:
    void logMessage(const QString &message);

private:
    static bool mEnabled;
    static Log *mInstance;
};

#endif // LOG_H
