#include "validatedelegate.h"

ValidateDelegate::ValidateDelegate(QObject *parent):QItemDelegate(parent)
{

}

// editing
QWidget *ValidateDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QRegExp regExp("[a-fA-F0-9]*");
    QLineEdit* editor = new QLineEdit(parent);
    editor->setValidator(new QRegExpValidator(regExp, parent));
    return editor;
}
