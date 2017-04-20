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

#ifndef UPDATE_H
#define UPDATE_H

#include <QString>

#ifdef Q_OS_DARWIN
#define SJ_UPDATE_URL "https://proxy.sh/safechecker_mac.xml"
#endif

#ifdef Q_OS_WIN
#define SJ_UPDATE_URL "https://proxy.sh/safechecker_windows.xml"
#endif

#ifdef Q_OS_LINUX
#define SJ_UPDATE_URL "https://proxy.sh/safechecker_linux.xml"
#endif

#endif // UPDATE_H
