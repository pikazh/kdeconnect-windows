#include "transferhistorylistmodel.h"
#include "ui/uicommon.h"

#include <QDateTime>
#include <QFileInfo>

TransferHistoryListModel::TransferHistoryListModel(QObject *parent)
    : QAbstractListModel{parent}
    , m_columns{Type, Result, File, FinishTime}
{}

size_t TransferHistoryListModel::appendData(const QList<TransferHistoryRecord> &records)
{
    if (records.isEmpty())
        return 0;

    qint64 oldestFinishTimeInCurrentRecords;
    QSet<qint64> recordIdsWithTheSameOldestFinishedTime;
    if (!m_records.isEmpty()) {
        auto it = m_records.crbegin();
        recordIdsWithTheSameOldestFinishedTime.insert(it->recordId);
        oldestFinishTimeInCurrentRecords = it->finishTime;
        for (++it; it != m_records.crend(); ++it) {
            if (it->finishTime == oldestFinishTimeInCurrentRecords)
                recordIdsWithTheSameOldestFinishedTime.insert(it->recordId);
            else
                break;
        }
    }

    QList<TransferHistoryRecord> filteredRecords = records;
    if (!recordIdsWithTheSameOldestFinishedTime.isEmpty()) {
        for (auto it = filteredRecords.begin(); it != filteredRecords.end();) {
            if (it->finishTime == oldestFinishTimeInCurrentRecords) {
                if (recordIdsWithTheSameOldestFinishedTime.contains(it->recordId)) {
                    it = filteredRecords.erase(it);
                } else {
                    ++it;
                }
            } else {
                break;
            }
        }
    }

    if (!filteredRecords.isEmpty()) {
        beginInsertRows(QModelIndex(),
                        m_records.size(),
                        m_records.size() + filteredRecords.size() - 1);
        for (auto it = filteredRecords.begin(); it != filteredRecords.end(); ++it)
            m_recordIds.insert(it->recordId);
        m_records.append(std::move(filteredRecords));
        endInsertRows();
    }

    return filteredRecords.size();
}

void TransferHistoryListModel::onAdded(qint64 id, const TransferHistoryRecord &r)
{
    Q_ASSERT(!m_recordIds.contains(id));

    beginInsertRows(QModelIndex(), 0, 0);
    m_records.prepend(r);
    m_recordIds.insert(id);
    endInsertRows();
}

void TransferHistoryListModel::onRemoved(qint64 id)
{
    if (m_recordIds.contains(id)) {
        for (auto it = m_records.constBegin(); it != m_records.constEnd(); ++it) {
            if (it->recordId == id) {
                auto index = it - m_records.constBegin();
                beginRemoveRows(QModelIndex(), index, index);
                m_records.erase(it);
                m_recordIds.remove(id);
                endRemoveRows();

                return;
            }
        }

        Q_ASSERT(false);
    }
}

void TransferHistoryListModel::onClear()
{
    beginResetModel();
    m_recordIds.clear();
    m_records.clear();
    endResetModel();
}

QVariant TransferHistoryListModel::headerData(int section,
                                              Qt::Orientation orientation,
                                              int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (m_columns[section]) {
        case Type:
            return tr("Type");
        case Result:
            return tr("Result");
        case File:
            return tr("File");
        case FinishTime:
            return tr("Finish Time");
        }
    }

    return QAbstractListModel::headerData(section, orientation, role);
}

int TransferHistoryListModel::columnCount(const QModelIndex &parent) const
{
    return m_columns.size();
}

int TransferHistoryListModel::rowCount(const QModelIndex &parent) const
{
    return m_records.size();
}

QVariant TransferHistoryListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    int row = index.row();
    switch (role) {
    case Qt::DisplayRole: {
        auto &record = m_records.at(row);
        int column = index.column();
        switch (column) {
        case Type:
            if (record.transferType == TransferHistoryRecord::TransferType::Receiving) {
                return tr("Receive File");
            } else {
                return tr("Send File");
            }
        case Result:
            if (record.result == TransferHistoryRecord::Result::SuccessFul) {
                return tr("Successful");
            } else if (record.result == TransferHistoryRecord::Result::Failed) {
                return tr("Failed");
            } else if (record.result == TransferHistoryRecord::Result::Aborted) {
                return tr("Aborted");
            } else {
                Q_ASSERT(false);
                return {};
            }
        case File:
            return QFileInfo(record.filePath).fileName();
        case FinishTime:
            return QDateTime::fromSecsSinceEpoch(record.finishTime).toString(Qt::ISODate);
        }

        break;
    }
    case Qt::ToolTipRole: {
        int column = index.column();
        if (column == File) {
            return m_records.at(row).filePath;
        }

        break;
    }
    case TransferHistoryListItemDataRoles::Id:
        return m_records.at(row).recordId;
    case TransferHistoryListItemDataRoles::Type:
        return m_records.at(row).transferType;
    case TransferHistoryListItemDataRoles::File:
        return m_records.at(row).filePath;
    case TransferHistoryListItemDataRoles::FinishTime:
        return m_records.at(row).finishTime;
    case TransferHistoryListItemDataRoles::Result:
        return m_records.at(row).result;
    case TransferHistoryListItemDataRoles::FailedReason:
        return m_records.at(row).failedReason;
    }

    return {};
}
