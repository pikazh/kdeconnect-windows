#include "smslistitemsortfilterproxymodel.h"
#include "uicommon.h"

SmsListItemSortFilterProxyModel::SmsListItemSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{}

bool SmsListItemSortFilterProxyModel::lessThan(const QModelIndex &source_left,
                                               const QModelIndex &source_right) const
{
    SmsListItemData::Ptr leftData = qvariant_cast<SmsListItemData::Ptr>(
        sourceModel()->data(source_left, SmsListItemDataRoles::Data));
    SmsListItemData::Ptr rightData = qvariant_cast<SmsListItemData::Ptr>(
        sourceModel()->data(source_right, SmsListItemDataRoles::Data));

    return leftData->latestMsgTime < rightData->latestMsgTime;
}
