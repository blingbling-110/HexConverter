#ifndef READONLYDELEGATE_H
#define READONLYDELEGATE_H

#include <QItemDelegate>

class ReadOnlyDelegate : public QItemDelegate
{
public:
    ReadOnlyDelegate(QObject *parent = Q_NULLPTR);

    // editing
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
};

#endif // READONLYDELEGATE_H
