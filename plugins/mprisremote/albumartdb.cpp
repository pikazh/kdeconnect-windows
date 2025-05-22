#include "albumartdb.h"
#include "plugin_mprisremote_debug.h"

#include <QDateTime>

AlbumArtDB::AlbumArtDB(QObject *parent)
    : QObject(parent)
{}

AlbumArtDB::~AlbumArtDB()
{
    unInit();
}

bool AlbumArtDB::init(const QString &connectionName, const QString &dbFilePath)
{
    m_db = QSqlDatabase::addDatabase(QLatin1String("QSQLITE"), connectionName);
    m_db.setDatabaseName(dbFilePath);
    if (!m_db.open()) {
        qCCritical(KDECONNECT_PLUGIN_MPRISREMOTE)
            << "open db" << dbFilePath << "failed with err" << m_db.lastError();
        return false;
    }
    QSqlQuery query(m_db);
    QString createTableSql = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS Records"
        "(Id INTEGER PRIMARY KEY AUTOINCREMENT, Url "
        "VARCHAR(256), CreationTime INTEGER, LastAccessTime INTEGER, Data BLOB)");
    if (!query.exec(createTableSql)) {
        qCCritical(KDECONNECT_PLUGIN_MPRISREMOTE)
            << "create table failed with err" << query.lastError();
        return false;
    }

    return true;
        
}

void AlbumArtDB::unInit()
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

bool AlbumArtDB::insert(const QString &url, const QByteArray &data)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("DELETE From Records WHERE Url = (:url)"));
    query.bindValue(":url", url);
    if (!query.exec()) {
        qCCritical(KDECONNECT_PLUGIN_MPRISREMOTE)
            << "delete record failed with err" << query.lastError();

        return false;
    }

    query.prepare(QStringLiteral("INSERT INTO Records(Url,CreationTime,LastAccessTime,Data) VALUES "
                                 "(:url,:time,:time,:data)"));
    query.bindValue(":url", url);
    query.bindValue(":time", QDateTime::currentSecsSinceEpoch());
    query.bindValue(":data", data);
    if (!query.exec()) {
        qCCritical(KDECONNECT_PLUGIN_MPRISREMOTE)
            << "insert record failed with err" << query.lastError();

        return false;
    }

    QMetaObject::invokeMethod(this, &AlbumArtDB::clean, Qt::QueuedConnection);

    return true;
}

bool AlbumArtDB::query(const QString &url, QByteArray &data)
{
    QSqlQuery query(m_db);

    query.prepare(QStringLiteral("SELECT CreationTime, Data From Records WHERE Url = (:url)"));
    query.bindValue(":url", url);
    if (query.exec() && query.next()) {
        qint64 diffTime = QDateTime::currentSecsSinceEpoch() - query.value(0).toInt();
        // make the record expired if it was created one week ago
        const qint64 maxAllowedDiffTime = 60 * 60 * 24 * 7;
        if (diffTime < 0 || diffTime > maxAllowedDiffTime) {
            return false;
        }

        data = std::move(query.value(1).toByteArray());

        query.prepare(
            QStringLiteral("Update Records SET LastAccessTime = (:time) WHERE Url = (:url)"));
        query.bindValue(":time", QDateTime::currentSecsSinceEpoch());
        query.bindValue(":url", url);
        return query.exec();
    } else {
        return false;
    }
}

bool AlbumArtDB::clean()
{
    QSqlQuery query(m_db);

    if (query.exec(QStringLiteral("SELECT COUNT(Id) From Records")) && query.next()) {
        int recordCnt = query.value(0).toInt();
        bool ret = true;
        if (recordCnt > m_limitedRecordCount) {
            ret = query.exec(
                QStringLiteral(
                    "DELETE FROM Records WHERE Records.LastAccessTime IN ("
                    "SELECT LastAccessTime FROM Records ORDER BY LastAccessTime ASC LIMIT %1)")
                    .arg(recordCnt - m_limitedRecordCount));
        }

        return ret;
    }

    return false;
}
