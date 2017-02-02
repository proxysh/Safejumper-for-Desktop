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

#include "lvrowdelegate.h"

#include <QPen>
#include <QPainter>
#include <QPicture>

#include "log.h"
#include "authmanager.h"
#include "setting.h"
#include "scr_map.h"

void LvRowDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option,
                          const QModelIndex & index) const
{
    QStyledItemDelegate::paint(painter, option, index);
    painter->save();

    // on -1 will return empty AServer
    int id = index.row() - 1;

    int idsrv = -1;
//	bool nodes = Setting::Instance()->IsShowNodes();
//	if (id > -1)
//		idsrv = nodes ? id : AuthManager::Instance()->ServerIdFromHubId(id);
    if (Scr_Map::IsExists() && id > -1) {
        idsrv = Scr_Map::Instance()->SrvIxFromLineIx(id);
    }
    AServer sr = AuthManager::instance()->getServer(idsrv);	//AServer sr = nodes ? 	AuthManager::Instance()->GetSrv(id) : AuthManager::Instance()->GetHub(id);

    painter->setRenderHint(QPainter::Antialiasing, true);

    static int w_checkmark = 25;
    static int margin_left = 5;		// padding-left: 10px;
    int w_ping = 55;
    int w_load = 40;

    bool selected = option.state & QStyle::State_Selected;
    bool checked = !(option.state & QStyle::State_Selected);
    if (Scr_Map::IsExists())
        checked = checked && (Scr_Map::Instance()->CurrSrv() == idsrv);

    if (checked && !selected && id > -1) {
        static QString sthubs = ":/imgs/dd-selectionrow-244.png";
        static QString stsrvs = ":/imgs/dd-selectionrow-322.png";
        QPixmap pm(Scr_Map::Instance()->UseSrvColl() ? stsrvs : sthubs);
        painter->drawPixmap(option.rect, pm);
    }

    static QPen white(QColor("#FFFFFF"));
    if (!selected) {
        QRect L = option.rect.adjusted(margin_left, 0, -1 * (w_ping + w_load + w_checkmark), 0);
        if (checked && id > -1)
            painter->setPen(white);
        painter->drawText(L, Qt::AlignLeft | Qt::AlignVCenter, id < 0 ? "-- Select location --": sr.name);
    }

    if (id > -1) {
        int load = (int)sr.load.toDouble();
        QString sload = QString::number(load) + " %";

        if (selected || checked)
            painter->setPen(white);
        else
            SetColor(painter, load, 25, 75);

        QRect R = option.rect.adjusted(0, 0, -1 * w_checkmark, 0);
        R.setLeft(R.right() - w_load);
        painter->drawText(R, Qt::AlignRight | Qt::AlignVCenter, sload);


        int ping = AuthManager::instance()->pingFromServerIx(idsrv);
        if (ping > -1) {
            if (selected || checked)
                painter->setPen(white);
            else
                SetColor(painter, ping, 100, 300);
            QRect R2 = option.rect.adjusted(0, 0, -1 * (w_checkmark + w_load), 0);
            R.setLeft(R.right() - w_ping);
            QString sping = QString::number(ping) + " ms";
            painter->drawText(R2, Qt::AlignRight | Qt::AlignVCenter, sping);
        }
    }

    painter->restore();
}


QSize LvRowDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize sz = QStyledItemDelegate::sizeHint(option, index);
    if (sz.width() < 300)
        sz.setWidth(300);
    return sz;
}

void LvRowDelegate::SetColor(QPainter * painter, int value, int threshold1, int threshold2) const
{
    static QPen green(QColor("#67ba52"));		// 009-UI-Kit-v1.psd
    static QPen red(QColor("#ee5b34"));
    static QPen grey(QColor("#abbcbc"));

    if (value < threshold1)
        painter->setPen(green);
    else if (value >= threshold2)
        painter->setPen(red);
    else
        painter->setPen(grey);
}
