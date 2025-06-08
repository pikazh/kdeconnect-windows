#pragma once

#include <QList>
#include <QString>

#include "itransferhistorychangelistener.h"
#include "transferhistoryrecord.h"

class ITransferHistoryManager
{
public:
    virtual void addHistory(int transferType,
                            const QString &filePath,
                            qint64 finishTime,
                            int result,
                            const QString &failedReason = {})
        = 0;

    virtual QList<TransferHistoryRecord> getHistories(qint64 finishTimeBegin, int count = 50) = 0;

    virtual void removeHistory(qint64 recordId) = 0;
    virtual void clearHistories() = 0;

    virtual void registerHistoryChangeListener(ITransferHistoryChangeListener *listener) = 0;
    virtual void unRegisterHistoryChangeListener(ITransferHistoryChangeListener *listener) = 0;
};
