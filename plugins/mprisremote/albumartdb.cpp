#include "albumartdb.h"
#include "plugin_mprisremote_debug.h"

#include <QDateTime>
#include <QDir>

AlbumArtDB::AlbumArtDB(QObject *parent)
    : QObject(parent)
    , m_db(QSqlDatabase::addDatabase(QLatin1String("QSQLITE")))
{}

AlbumArtDB::~AlbumArtDB()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    QSqlDatabase::removeDatabase(QLatin1String("QSQLITE"));
}

bool AlbumArtDB::init(const QString &dbDir)
{
    QDir d(dbDir);
    d.mkpath(QStringLiteral("."));
    QString dbPath = dbDir + QDir::separator() + QStringLiteral("albumart");
    m_db.setDatabaseName(dbPath);
    if (!m_db.open()) {
        qWarning(KDECONNECT_PLUGIN_MPRISREMOTE)
            << "open db" << dbPath << "failed with err" << m_db.lastError();
        return false;
    }
    QSqlQuery query(m_db);
    QString createTableSql = QStringLiteral("CREATE TABLE IF NOT EXISTS Records"
                                            "(Id INTEGER PRIMARY KEY AUTOINCREMENT, Url "
                                            "VARCHAR(256), LastAccessTime INTEGER, Data BLOB)");
    if (!query.exec(createTableSql)) {
        qWarning(KDECONNECT_PLUGIN_MPRISREMOTE)
            << "create table failed with err" << query.lastError();
        return false;
    }

    return true;
        
}

bool AlbumArtDB::insert(const QString &url, const QByteArray &data)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT Id From Records WHERE Url = (:url)"));
    query.bindValue(":url", url);
    if (query.exec() && query.next()) {
        query.prepare(QStringLiteral("DELETE From Records WHERE Url = (:url)"));
        query.bindValue(":url", url);
        if (!query.exec()) {
            qWarning(KDECONNECT_PLUGIN_MPRISREMOTE)
                << "delete record failed with err" << query.lastError();

            return false;
        }
    }

    query.prepare(
        QStringLiteral("INSERT INTO Records(Url,LastAccessTime,Data) VALUES (:url,:time,:data)"));
    query.bindValue(":url", url);
    query.bindValue(":time", QDateTime::currentSecsSinceEpoch());
    query.bindValue(":data", data);
    if (!query.exec()) {
        qWarning(KDECONNECT_PLUGIN_MPRISREMOTE)
            << "insert record failed with err" << query.lastError();

        return false;
    }

    QMetaObject::invokeMethod(this, &AlbumArtDB::clean, Qt::QueuedConnection);

    return true;
}

bool AlbumArtDB::query(const QString &url, QByteArray &data)
{
    QSqlQuery query(m_db);

    query.prepare(QStringLiteral("SELECT Data From Records WHERE Url = (:url)"));
    query.bindValue(":url", url);
    if (query.exec() && query.next()) {
        data = std::move(query.value(0).toByteArray());

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
