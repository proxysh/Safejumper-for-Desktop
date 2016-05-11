#ifndef LVROWDELEGATEENCRYPTION_H
#define LVROWDELEGATEENCRYPTION_H

#include <QStyledItemDelegate>

class LvRowDelegateEncryption : public QStyledItemDelegate
{
	Q_OBJECT
public:
	explicit LvRowDelegateEncryption(QObject *parent = 0) : QStyledItemDelegate(parent) {}

	void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
};

#endif // LVROWDELEGATEENCRYPTION_H
