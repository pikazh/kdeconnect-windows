#pragma once

#include "kdeconnectcore_export.h"

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QSharedDataPointer>

class PluginMetaDataPrivate;

class KDECONNECTCORE_EXPORT PluginMetaData
{
    Q_GADGET
    Q_PROPERTY(bool isValid READ isValid CONSTANT)
    Q_PROPERTY(QJsonObject rawData READ rawData CONSTANT)
    Q_PROPERTY(QString filePath READ filePath CONSTANT)
    Q_PROPERTY(QString fileName READ fileName CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QString iconName READ iconName CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(bool isEnabledByDefault READ isEnabledByDefault CONSTANT)
public:
    PluginMetaData();
    PluginMetaData(const QString &pluginFilePath);
    PluginMetaData(const PluginMetaData &);
    PluginMetaData &operator=(const PluginMetaData &);
    ~PluginMetaData();

    static QList<PluginMetaData> load();

    bool isValid() const;
    QJsonObject rawData() const;
    QString filePath() const;
    QString fileName() const;
    QString name() const;
    QString id() const;
    QString description() const;
    QString iconName() const;
    QString version() const;
    bool isEnabledByDefault() const;

    QString value(QStringView key, const QString &defaultValue = QString()) const;
    QStringList value(QStringView key, const QStringList &defaultValue) const;
    int value(QStringView key, int defaultValue) const;
    bool value(QStringView key, bool defaultValue) const;

private:
    QExplicitlySharedDataPointer<PluginMetaDataPrivate> d;
};
