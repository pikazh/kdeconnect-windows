#pragma once

#include <QStyledItemDelegate>
#include <QWidget>

class RemoteCommandsListItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit RemoteCommandsListItemDelegate(QWidget *parent = nullptr);

protected:
    virtual void paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;
};
