#ifndef LVROWDELEGATEPROTOCOL_H
#define LVROWDELEGATEPROTOCOL_H

#include <QStyledItemDelegate>

class LvRowDelegateProtocol : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit LvRowDelegateProtocol(QObject *parent = 0) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;
};

#endif // LVROWDELEGATEPROTOCOL_H
