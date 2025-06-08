#include "transferhistorymanager.h"

#include "core/kdeconnectconfig.h"

#include <QVariant>

TransferHistoryManager::TransferHistoryManager(const QString &deviceId, QObject *parent)
    : QObject{parent}
    , m_db(new TransferHistoryDB(this))
{
    QString dbPath = KdeConnectConfig::instance()->deviceDataDir(deviceId).absolutePath();
    QDir().mkpath(dbPath);
    dbPath = dbPath + QDir::separator() + QStringLiteral("transfers");
    QString connectionName = deviceId + QStringLiteral("/transfers");
    m_db->init(connectionName, dbPath);
}

void TransferHistoryManager::addHistory(int transferType,
                                        const QString &filePath,
                                        qint64 finishTime,
                                        int result,
                                        const QString &failedReason)
{
    qint64 recordId = m_db->insert(transferType, filePath, finishTime, result, failedReason);
    if (recordId > -1) {
        TransferHistoryRecord record;
        record.transferType = transferType;
        record.filePath = filePath;
        record.finishTime = finishTime;
        record.result = result;
        record.failedReason = failedReason;
        record.recordId = recordId;

        notifyListenersOnHistoryAdded(recordId, record);
    }
}

QList<TransferHistoryRecord> TransferHistoryManager::getHistories(qint64 finishTimeBegin, int count)
{
    return m_db->query(finishTimeBegin, count);
}

void TransferHistoryManager::removeHistory(qint64 recordId)
{
    if (m_db->remove(recordId)) {
        notifyListenersOnHistoryRemoved(recordId);
    }
}

void TransferHistoryManager::clearHistories()
{
    if (m_db->clear()) {
        notifyListenersOnHistoryClear();
    }
}

void TransferHistoryManager::registerHistoryChangeListener(ITransferHistoryChangeListener *listener)
{
    if (m_listeners.indexOf(listener) == -1) {
        m_listeners.append(listener);
    }
}

void TransferHistoryManager::unRegisterHistoryChangeListener(ITransferHistoryChangeListener *listener)
{
    m_listeners.removeAll(listener);
}

void TransferHistoryManager::notifyListenersOnHistoryAdded(qint64 recordId,
                                                           const TransferHistoryRecord &r)
{
    for (auto listener : m_listeners) {
        listener->onAdded(recordId, r);
    }
}

void TransferHistoryManager::notifyListenersOnHistoryRemoved(qint64 recordId)
{
    for (auto listener : m_listeners) {
        listener->onRemoved(recordId);
    }
}

void TransferHistoryManager::notifyListenersOnHistoryClear()
{
    for (auto listener : m_listeners) {
        listener->onClear();
    }
}
