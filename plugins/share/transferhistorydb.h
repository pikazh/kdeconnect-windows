#pragma once

#include <QObject>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <QByteArray>
#include <QString>

#include "core/plugins/share/transferhistoryrecord.h"

class TransferHistoryDB : public QObject
{
    Q_OBJECT
public:
    //Q_DECLARE_METATYPE(TransferHistoryDB)

    explicit TransferHistoryDB(QObject *parent = nullptr);
    virtual ~TransferHistoryDB() override;

public:
    bool init(const QString &connectionName, const QString &dbFilePath);
    void unInit();

    qint64 insert(int transferType,
                  const QString &filePath,
                  qint64 finishTime,
                  int result,
                  const QString &failedReason = {});
    bool remove(qint64 recordId);
    QList<TransferHistoryRecord> query(qint64 finishTimeBegin = -1, int count = 50);
    bool clear();

private:
    QSqlDatabase m_db;
};
