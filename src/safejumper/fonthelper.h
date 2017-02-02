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

#ifndef FONTHELPER_H
#define FONTHELPER_H

#include <memory>

#include <QFont>
#include <QWidget>

//	FontHelper::SetFont(this);
class FontHelper
{
public:
    static QFont & font();
    static QFont & bold();
    static QFont pt(size_t pt, bool bold = false);
    static void SetFont(QWidget * w);
private:
    static void Init();
    static std::auto_ptr<QFont> _f;
    static std::auto_ptr<QFont> _bold;
};

#endif // FONTHELPER_H
