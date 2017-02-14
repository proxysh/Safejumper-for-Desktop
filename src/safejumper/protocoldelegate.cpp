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

#include "protocoldelegate.h"
#include "mapscreen.h"
#include "setting.h"

#include <QPen>
#include <QPainter>
#include <QPicture>

void ProtocolDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option,
                                  const QModelIndex & index) const
{
    QStyledItemDelegate::paint(painter, option, index);
    painter->save();

    int id = index.row() - 1;
    bool selected = option.state & QStyle::State_Selected;
    bool checked = !(option.state & QStyle::State_Selected);
    checked = checked && (Setting::instance()->currentProtocol() == id);

    if (checked && !selected && id > -1) {
        static QPixmap pm(":/imgs/dd-selectionrow.png");
        painter->drawPixmap(option.rect, pm);

        static QPen white(QColor("#FFFFFF"));
        static int margin_left = 5;		// padding-left: 10px;
        QRect L = option.rect.adjusted(margin_left, 0, 0, 0);
        painter->setPen(white);
        painter->drawText(L, Qt::AlignLeft | Qt::AlignVCenter, id < 0 ? PROTOCOL_SELECTION_STR : Setting::instance()->currentEncryptionProtocols().at(id));
    }

    painter->restore();
}
