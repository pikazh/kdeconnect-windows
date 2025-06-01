#pragma once

#include <QStyledItemDelegate>

class SMSConversationListItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    SMSConversationListItemDelegate(QWidget *parent);

protected:
    virtual void paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;
};
