#pragma once

#include "core/plugins/share/itransferhistorychangelistener.h"
#include "core/plugins/share/transferhistoryrecord.h"

#include <QAbstractListModel>
#include <QList>
#include <QSet>

class TransferHistoryListModel : public QAbstractListModel, public ITransferHistoryChangeListener
{
    Q_OBJECT
public:
    enum Column { Type = 0, Result, File, FinishTime };
    explicit TransferHistoryListModel(QObject *parent = nullptr);

    size_t appendData(const QList<TransferHistoryRecord> &records);

    virtual void onAdded(qint64 id, const TransferHistoryRecord &r) override;
    virtual void onRemoved(qint64 id) override;
    virtual void onClear() override;

    virtual QVariant headerData(int section,
                                Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QList<TransferHistoryRecord> m_records;
    QSet<qint64> m_recordIds;
    QList<Column> m_columns;
};
