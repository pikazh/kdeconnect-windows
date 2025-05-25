#pragma once

#include <QSortFilterProxyModel>

class SmsListItemSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    SmsListItemSortFilterProxyModel(QObject *parent = nullptr);

protected:
    virtual bool lessThan(const QModelIndex &source_left,
                          const QModelIndex &source_right) const override;
};
