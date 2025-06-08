#pragma once

#include "transferhistorydb.h"

#include "core/plugins/share/itransferhistorymanager.h"

#include <QList>
#include <QObject>

class TransferHistoryManager : public QObject, public ITransferHistoryManager
{
    Q_OBJECT
public:
    explicit TransferHistoryManager(const QString &deviceId, QObject *parent = nullptr);

    virtual void addHistory(int transferType,
                            const QString &filePath,
                            qint64 finishTime,
                            int result,
                            const QString &failedReason = {}) override;

    virtual QList<TransferHistoryRecord> getHistories(qint64 finishTimeBegin = -1,
                                                      int count = 50) override;

    virtual void removeHistory(qint64 recordId) override;
    virtual void clearHistories() override;

    virtual void registerHistoryChangeListener(ITransferHistoryChangeListener *listener) override;
    virtual void unRegisterHistoryChangeListener(ITransferHistoryChangeListener *listener) override;

protected:
    void notifyListenersOnHistoryAdded(qint64 recordId, const TransferHistoryRecord &r);
    void notifyListenersOnHistoryRemoved(qint64 recordId);
    void notifyListenersOnHistoryClear();

private:
    TransferHistoryDB *m_db;
    QList<ITransferHistoryChangeListener *> m_listeners;
};
