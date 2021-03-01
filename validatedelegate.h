#ifndef VALIDATEDELEGATE_H
#define VALIDATEDELEGATE_H

#include <QItemDelegate>
#include <QLineEdit>

class ValidateDelegate : public QItemDelegate
{
public:
    ValidateDelegate(QObject *parent = Q_NULLPTR);

    // editing
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
};

#endif // VALIDATEDELEGATE_H
