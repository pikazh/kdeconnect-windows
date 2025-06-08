#pragma once

#include <QString>
#include <QtTypes>

struct TransferHistoryRecord
{
    enum TransferType {
        Receiving = 0,
        Sending,
    };

    enum Result {
        SuccessFul = 0,
        Failed,
        Aborted,
        Unknown,
    };

    qint64 recordId;
    int transferType;
    QString filePath;
    qint64 finishTime;
    int result;
    QString failedReason;
};
