#ifndef LVROWDELEGATE_H
#define LVROWDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
//#include <QItemDelegate>



class LvRowDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	explicit LvRowDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) {}

	void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
	void SetColor(QPainter * painter, int value, int threshold1, int threshold2) const;
};


#endif // LVROWDELEGATE_H
