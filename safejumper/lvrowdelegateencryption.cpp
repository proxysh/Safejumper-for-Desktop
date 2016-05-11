#include "lvrowdelegateencryption.h"

#include <QPen>
#include <QPainter>
#include <QPicture>

#include "setting.h"

void LvRowDelegateEncryption::paint(QPainter * painter, const QStyleOptionViewItem & option,
            const QModelIndex & index) const
{
	QStyledItemDelegate::paint(painter, option, index);
	painter->save();

	int enc = index.row();
	bool selected = option.state & QStyle::State_Selected;
	bool checked = !(option.state & QStyle::State_Selected);
	if (Setting::IsExists())
		checked = checked && (Setting::Instance()->Encryption() == enc);

	if (checked && !selected && enc > -1)
	{
		static QPixmap pm(":/imgs/dd-selectionrow.png");		// 210 pt
		painter->drawPixmap(option.rect, pm);

		static QPen white(QColor("#FFFFFF"));
		static int margin_left = 5;		// padding-left: 10px;
		QRect L = option.rect.adjusted(margin_left, 0, 0, 0);
		painter->setPen(white);
		painter->drawText(L, Qt::AlignLeft | Qt::AlignVCenter, Setting::EncText(enc));
	}

	painter->restore();
}

QSize LvRowDelegateEncryption::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QSize sz = QStyledItemDelegate::sizeHint(option, index);
//	if (sz.width() < 210)
//		sz.setWidth(210);
	return sz;
}



