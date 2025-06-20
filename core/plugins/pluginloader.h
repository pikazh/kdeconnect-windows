#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include "pluginmetadata.h"
#include "kdeconnectcore_export.h"

#include <QHash>
#include <QObject>
#include <QString>

class Device;
class KdeConnectPlugin;
class KPluginFactory;

class KDECONNECTCORE_EXPORT PluginLoader
{
public:
    static PluginLoader *instance();

    QStringList getPluginList() const;
    bool doesPluginExist(const QString &id) const;
    PluginMetaData getPluginInfo(const QString &id) const;
    KdeConnectPlugin *instantiatePluginForDevice(const QString &pluginId, Device *device) const;

    QStringList incomingCapabilities() const;
    QStringList outgoingCapabilities() const;
    QSet<QString> pluginsForCapabilities(const QSet<QString> &incoming, const QSet<QString> &outgoing) const;

private:
    PluginLoader();

    QHash<QString, PluginMetaData> plugins;
};

#endif
