#pragma once

#include <QObject>

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QString>

#include <utility>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

class ContactsDB : public QObject
{
    Q_OBJECT
public:
    explicit ContactsDB(QObject *parent = nullptr);
    virtual ~ContactsDB() override;

    bool init(const QString &connectionName, const QString &dbFilePath);
    void unInit();

    QList<std::tuple<QString, qint64, QByteArray>> allData();

    QHash<QString, qint64> allUidAndTimeStamps();
    bool deleteRecords(QList<QString> uids);
    bool insertOrUpdateRecord(const QString &uId, const qint64 timeStamp, const QByteArray &data);

private:
    QSqlDatabase m_db;
};
