#pragma once

#include <Qt>

#include <QList>
#include <QSharedPointer>
#include <QString>

enum SmsListItemDataRoles { Data = Qt::UserRole + 1 };

struct SmsListItemData
{
    qint64 conversationId = -1;
    QList<QString> canonicalizedPhoneNumbers;
    qint64 latestMsgTime = -1;
    int conversationContentWidgetIndex = -1;

    using Ptr = QSharedPointer<SmsListItemData>;
};
Q_DECLARE_METATYPE(SmsListItemData)
