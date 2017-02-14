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

#include "encryptiondelegate.h"

#include <QPen>
#include <QPainter>
#include <QPicture>

#include "setting.h"

void EncryptionDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option,
                                    const QModelIndex & index) const
{
    QStyledItemDelegate::paint(painter, option, index);
    painter->save();

    int enc = index.row();
    bool selected = option.state & QStyle::State_Selected;
    bool checked = !(option.state & QStyle::State_Selected);
    if (Setting::exists())
        checked = checked && (Setting::instance()->encryption() == enc);

    if (checked && !selected && enc > -1) {
        static QPixmap pm(":/imgs/dd-selectionrow.png");		// 210 pt
        painter->drawPixmap(option.rect, pm);

        static QPen white(QColor("#FFFFFF"));
        static int margin_left = 5;		// padding-left: 10px;
        QRect L = option.rect.adjusted(margin_left, 0, 0, 0);
        painter->setPen(white);
        painter->drawText(L, Qt::AlignLeft | Qt::AlignVCenter, Setting::encryptionName(enc));
    }

    painter->restore();
}

QSize EncryptionDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize sz = QStyledItemDelegate::sizeHint(option, index);
//	if (sz.width() < 210)
//		sz.setWidth(210);
    return sz;
}



