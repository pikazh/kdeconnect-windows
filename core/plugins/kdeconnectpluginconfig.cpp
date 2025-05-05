/**
 * SPDX-FileCopyrightText: 2015 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QDebug>
#include <QDir>
#include <QSettings>

#include "kdeconnectpluginconfig.h"
#include "kdeconnectconfig.h"

struct KdeConnectPluginConfigPrivate {
    QDir m_configDir;
    QSettings *m_config;
};

KdeConnectPluginConfig::KdeConnectPluginConfig(const QString &deviceId,
                                               const QString &pluginId,
                                               QObject *parent)
    : QObject(parent)
    , d(new KdeConnectPluginConfigPrivate())
{
    d->m_configDir = KdeConnectConfig::instance().pluginConfigDir(deviceId, pluginId);
    QDir().mkpath(d->m_configDir.path());

    d->m_config = new QSettings(d->m_configDir.absoluteFilePath(QStringLiteral("config")), QSettings::IniFormat);
}

KdeConnectPluginConfig::~KdeConnectPluginConfig()
{
    delete d->m_config;
}

QString KdeConnectPluginConfig::getString(const QString &key, const QString &defaultValue)
{
    return d->m_config->value(key, defaultValue).toString();
}

bool KdeConnectPluginConfig::getBool(const QString &key, const bool defaultValue)
{
    return d->m_config->value(key, defaultValue).toBool();
}

int KdeConnectPluginConfig::getInt(const QString &key, const int defaultValue)
{
    return d->m_config->value(key, defaultValue).toInt();
}

QByteArray KdeConnectPluginConfig::getByteArray(const QString &key, const QByteArray defaultValue)
{
    return d->m_config->value(key, defaultValue).toByteArray();
}

QVariantList KdeConnectPluginConfig::getList(const QString &key, const QVariantList &defaultValue)
{
    QVariantList list;

    int size = d->m_config->beginReadArray(key);
    if (size < 1) {
        d->m_config->endArray();
        return defaultValue;
    }
    for (int i = 0; i < size; ++i) {
        d->m_config->setArrayIndex(i);
        list << d->m_config->value(QStringLiteral("value"));
    }
    d->m_config->endArray();
    return list;
}

void KdeConnectPluginConfig::set(const QString &key, const QVariant &value)
{
    d->m_config->setValue(key, value);
}

void KdeConnectPluginConfig::setList(const QString &key, const QVariantList &list)
{
    d->m_config->beginWriteArray(key);
    for (int i = 0; i < list.size(); ++i) {
        d->m_config->setArrayIndex(i);
        d->m_config->setValue(QStringLiteral("value"), list.at(i));
    }
    d->m_config->endArray();
}

void KdeConnectPluginConfig::notifyConfigChanged()
{
    Q_EMIT configChanged();
}
