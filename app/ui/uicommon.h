#pragma once

#include <Qt>

#include <QList>
#include <QSharedPointer>
#include <QString>

enum SmsListItemDataRoles { Data = Qt::UserRole + 1 };

enum RemoteCommandsListItemDataRoles { Name = Qt::UserRole + 1, Command, Key };

enum TransferHistoryListItemDataRoles {
    Id = Qt::UserRole + 1,
    Type,
    File,
    FinishTime,
    Result,
    FailedReason
};

struct SmsListItemData
{
    qint64 conversationId = -1;
    QList<QString> canonicalizedPhoneNumbers;
    qint64 latestMsgTime = -1;
    qint64 simcardSubId = -1;
    int conversationContentWidgetIndex = -1;

    using Ptr = QSharedPointer<SmsListItemData>;
};
Q_DECLARE_METATYPE(SmsListItemData)
