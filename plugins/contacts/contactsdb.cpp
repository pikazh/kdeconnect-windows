#include "contactsdb.h"
#include "plugin_contacts_debug.h"

ContactsDB::ContactsDB(QObject *parent)
    : QObject(parent)
{}

ContactsDB::~ContactsDB()
{
    unInit();
}

bool ContactsDB::init(const QString &connectionName, const QString &dbFilePath)
{
    m_db = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), connectionName);
    m_db.setDatabaseName(dbFilePath);
    if (!m_db.open()) {
        qCCritical(KDECONNECT_PLUGIN_CONTACTS)
            << "open db" << dbFilePath << "failed with err" << m_db.lastError();
        return false;
    }
    QSqlQuery query(m_db);
    QString createTableSql = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS Contacts"
        "(Uid VARCHAR(128) PRIMARY KEY NOT NULL, TimeStamp INTEGER NOT NULL, Data BLOB NOT NULL)");
    if (!query.exec(createTableSql)) {
        qCCritical(KDECONNECT_PLUGIN_CONTACTS)
            << "create table failed with err" << query.lastError();
        return false;
    }

    return true;
}

void ContactsDB::unInit()
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

QList<std::tuple<QString, qint64, QByteArray> > ContactsDB::allData()
{
    QList<std::tuple<QString, qint64, QByteArray> > rets;
    QSqlQuery query(m_db);
    if (query.exec(QStringLiteral("SELECT Uid, TimeStamp, Data From Contacts"))) {
        while (query.next()) {
            QString uid = qvariant_cast<QString>(query.value(0));
            qint64 timeStamp = qvariant_cast<qint64>(query.value(1));
            QByteArray data = qvariant_cast<QByteArray>(query.value(2));

            rets.push_back({uid, timeStamp, data});
        }
    }

    return rets;
}

QHash<QString, qint64> ContactsDB::allUidAndTimeStamps()
{
    QHash<QString, qint64> rets;
    QSqlQuery query(m_db);

    if (query.exec(QStringLiteral("SELECT Uid, TimeStamp From Contacts"))) {
        while (query.next()) {
            QString uid = qvariant_cast<QString>(query.value(0));
            qint64 timeStamp = qvariant_cast<qint64>(query.value(1));
            rets[uid] = timeStamp;
        }
    }

    return rets;
}

bool ContactsDB::deleteRecords(QList<QString> uids)
{
    if (uids.isEmpty())
        return true;

    QSqlQuery query(m_db);

    QList<QString> formattedUids = uids;
    for (auto it = formattedUids.begin(); it != formattedUids.end(); ++it) {
        *it = QStringLiteral("\"") + *it + QStringLiteral("\"");
    }

    QString uidsStr = formattedUids.join(',');
    if (!query.exec(
            QStringLiteral("DELETE FROM Contacts WHERE Contacts.Uid IN (%1)").arg(uidsStr))) {
        qCCritical(KDECONNECT_PLUGIN_CONTACTS)
            << "delete record failed with err" << query.lastError();

        return false;
    }

    return true;
}

bool ContactsDB::insertOrUpdateRecord(const QString &uId,
                                      const qint64 timeStamp,
                                      const QByteArray &data)
{
    if (uId.isEmpty() || timeStamp <= 0 || data.isEmpty()) {
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT Count(Uid) FROM Contacts WHERE Uid = (:uid)"));
    query.bindValue(":uid", uId);
    if (!(query.exec() && query.next())) {
        qCCritical(KDECONNECT_PLUGIN_CONTACTS)
            << "select query failed with err" << query.lastError();

        return false;
    }

    if (qvariant_cast<int>(query.value(0)) > 0) {
        query.prepare(QStringLiteral(
            "UPDATE Contacts SET TimeStamp = (:timestamp), Data = (:data) WHERE Uid = (:uid)"));
        query.bindValue(":timestamp", timeStamp);
        query.bindValue(":data", data);
        query.bindValue(":uid", uId);
        if (!query.exec()) {
            qCCritical(KDECONNECT_PLUGIN_CONTACTS) << "UPDATE failed with err" << query.lastError();

            return false;
        }
    } else {
        query.prepare(QStringLiteral(
            "INSERT INTO Contacts (Uid,TimeStamp,Data) VALUES (:uid,:timestamp,:data)"));
        query.bindValue(":timestamp", timeStamp);
        query.bindValue(":data", data);
        query.bindValue(":uid", uId);
        if (!query.exec()) {
            qCCritical(KDECONNECT_PLUGIN_CONTACTS) << "INSERT failed with err" << query.lastError();

            return false;
        }
    }

    return true;
}
