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

#ifndef FLAG_H
#define FLAG_H

#include <QString>
#include <QPoint>

class flag
{
public:
    static QString IconFromSrvName(const QString & srv);
    static QPoint CoordsFromSrvName(const QString & srv);

    // name from server into short name to display on map control
    // e.g.
    // U.S. Florida Hub => U.S.
    static QString ShortName(const QString & name);

    // clear name from "Hub", "Boost", and trailing server number
    // U.S. Florida Hub => U.S. Florida
    // Japan 2 => Japan
    static QString ClearName(const QString & name);
private:
    static int IdFromName(const QString & srv);
    static QString HandleTypo(const QString & name);                    // handle typo in server names: e.g. Brasil => Brazil
};

#endif // FLAG_H
