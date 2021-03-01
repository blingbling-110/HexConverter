#include "readonlydelegate.h"

ReadOnlyDelegate::ReadOnlyDelegate(QObject *parent):QItemDelegate(parent)
{

}

// editing
QWidget *ReadOnlyDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    Q_UNUSED(parent);
    Q_UNUSED(option);
    Q_UNUSED(index);
    return Q_NULLPTR;
}
