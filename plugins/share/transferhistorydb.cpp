#include "transferhistorydb.h"
#include "plugin_share_debug.h"

TransferHistoryDB::TransferHistoryDB(QObject *parent)
    : QObject{parent}
{}

TransferHistoryDB::~TransferHistoryDB()
{
    unInit();
}

bool TransferHistoryDB::init(const QString &connectionName, const QString &dbFilePath)
{
    m_db = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), connectionName);
    m_db.setDatabaseName(dbFilePath);
    if (!m_db.open()) {
        qCCritical(KDECONNECT_PLUGIN_SHARE)
            << "open db" << dbFilePath << "failed with err" << m_db.lastError();
        return false;
    }
    QSqlQuery query(m_db);
    QString createTableSql = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS TransferHistory"
        "(Id INTEGER PRIMARY KEY AUTOINCREMENT, TransferType INTEGER NOT NULL, "
        "File VARCHAR(256) NOT NULL, "
        "FinishTime INTEGER NOT NULL, Result INTEGER NOT NULL, FailedReason VARCHAR(256))");
    if (!query.exec(createTableSql)) {
        qCCritical(KDECONNECT_PLUGIN_SHARE) << "create table failed with err" << query.lastError();
        return false;
    }

    return true;
}

void TransferHistoryDB::unInit()
{
    if (m_db.isValid()) {
        if (m_db.isOpen()) {
            m_db.close();
        }
        QString connName = m_db.connectionName();
        m_db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connName);
    }
}

qint64 TransferHistoryDB::insert(int transferType,
                                 const QString &filePath,
                                 qint64 finishTime,
                                 int result,
                                 const QString &failedReason)
{
    QSqlQuery query(m_db);

    query.prepare(QStringLiteral(
        "INSERT INTO TransferHistory(TransferType,File,FinishTime,Result,FailedReason) VALUES "
        "(:transferType,:file,:time,:result,:failedReasion)"));
    query.bindValue(":transferType", transferType);
    query.bindValue(":file", filePath);
    query.bindValue(":time", finishTime);
    query.bindValue(":result", result);
    query.bindValue(":failedReasion", failedReason);
    if (!query.exec()) {
        qCCritical(KDECONNECT_PLUGIN_SHARE) << "insert record failed with err" << query.lastError();

        return -1;
    }

    if (!query.lastInsertId().canConvert<qint64>())
        return -1;

    auto insertId = qvariant_cast<qint64>(query.lastInsertId());

    return insertId;
}

bool TransferHistoryDB::remove(qint64 recordId)
{
    QSqlQuery query(m_db);
    return query.exec(QStringLiteral("DELETE FROM TransferHistory WHERE Id = %1").arg(recordId));
}

bool TransferHistoryDB::clear()
{
    QSqlQuery query(m_db);
    return query.exec(QStringLiteral("DELETE FROM TransferHistory"));
}

QList<TransferHistoryRecord> TransferHistoryDB::query(qint64 finishTimeBeginDesc, int count)
{
    QList<TransferHistoryRecord> rets;

    QSqlQuery query(m_db);
    if (finishTimeBeginDesc == -1) {
        query.prepare(QStringLiteral(
            "SELECT Id, TransferType, File, FinishTime, Result, FailedReason FROM TransferHistory"
            " ORDER BY FinishTime Desc LIMIT (:count)"));
        query.bindValue(":count", count);
    } else {
        query.prepare(QStringLiteral(
            "SELECT Id, TransferType, File, FinishTime, Result, FailedReason FROM TransferHistory"
            " WHERE FinishTime <= (:finishTime) ORDER BY FinishTime Desc LIMIT (:count)"));
        query.bindValue(":finishTime", finishTimeBeginDesc);
        query.bindValue(":count", count);
    }

    if (!query.exec()) {
        qCCritical(KDECONNECT_PLUGIN_SHARE) << "query failed with err" << query.lastError();
        return rets;
    }

    while (query.next()) {
        TransferHistoryRecord record;
        record.recordId = qvariant_cast<qint64>(query.value(0));
        int transferType = qvariant_cast<int>(query.value(1));
        record.transferType = (transferType == 1 ? TransferHistoryRecord::TransferType::Sending
                                                 : TransferHistoryRecord::TransferType::Receiving);
        record.filePath = query.value(2).toString();
        record.finishTime = qvariant_cast<qint64>(query.value(3));
        int result = qvariant_cast<int>(query.value(4));
        switch (result) {
        case TransferHistoryRecord::Result::SuccessFul:
        case TransferHistoryRecord::Result::Failed:
        case TransferHistoryRecord::Result::Aborted:
            record.result = result;
            break;
        default:
            record.result = TransferHistoryRecord::Result::Unknown;
            break;
        }

        record.failedReason = query.value(5).toString();

        rets.append(std::move(record));
    }

    return rets;
}
