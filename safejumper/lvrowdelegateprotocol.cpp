#include "lvrowdelegateprotocol.h"
#include "scr_map.h"
#include "setting.h"

#include <QPen>
#include <QPainter>
#include <QPicture>

void LvRowDelegateProtocol::paint(QPainter * painter, const QStyleOptionViewItem & option,
                                  const QModelIndex & index) const
{
    QStyledItemDelegate::paint(painter, option, index);
    painter->save();

    int id = index.row() - 1;
    bool selected = option.state & QStyle::State_Selected;
    bool checked = !(option.state & QStyle::State_Selected);
    if (Scr_Map::IsExists())
        checked = checked && (Scr_Map::Instance()->CurrProto() == id);

    if (checked && !selected && id > -1) {
        static QPixmap pm(":/imgs/dd-selectionrow.png");
        painter->drawPixmap(option.rect, pm);

        static QPen white(QColor("#FFFFFF"));
        static int margin_left = 5;		// padding-left: 10px;
        QRect L = option.rect.adjusted(margin_left, 0, 0, 0);
        painter->setPen(white);
        painter->drawText(L, Qt::AlignLeft | Qt::AlignVCenter, id < 0 ? PROTOCOL_SELECTION_STR : Setting::GetAllProt().at(id));
    }

    painter->restore();
}
