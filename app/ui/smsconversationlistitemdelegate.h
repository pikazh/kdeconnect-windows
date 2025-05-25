#pragma once

#include <QList>
#include <QString>
#include <QStyledItemDelegate>
#include <QTextLayout>

#include <utility>

class SMSConversationListItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    SMSConversationListItemDelegate(QWidget *parent);

protected:
    virtual void paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;

    QList<std::pair<qreal, QString>> viewItemTextLayout(QTextLayout &textLayout,
                                                        int lineWidth,
                                                        qreal &height) const;
};
