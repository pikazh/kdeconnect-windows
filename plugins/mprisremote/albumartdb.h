#pragma once

#include <QObject>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <QByteArray>
#include <QString>

class AlbumArtDB : public QObject
{
    Q_OBJECT
public:
    explicit AlbumArtDB(QObject *parent = nullptr);
    virtual ~AlbumArtDB() override;

    bool init(const QString &dbDir);

public Q_SLOTS:
    bool insert(const QString &url, const QByteArray &data);
    bool query(const QString &url, QByteArray &data);
    bool clean();

private:
    QSqlDatabase m_db;
    const int m_limitedRecordCount = 50;
};
