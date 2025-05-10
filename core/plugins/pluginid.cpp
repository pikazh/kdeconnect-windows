#include "pluginid.h"

#include <QHash>

QHash<PluginId, QString> initPluginNameHashTable()
{
    QHash<PluginId, QString> pluginNames;
    pluginNames.insert(PluginId::Ping, QStringLiteral("Ping"));
    pluginNames.insert(PluginId::BatteryMonitor, QStringLiteral("Battery monitor"));
    pluginNames.insert(PluginId::MprisRemote, QStringLiteral("MprisRemote"));
    pluginNames.insert(PluginId::MprisController, QStringLiteral("Multimedia control receiver"));
    pluginNames.insert(PluginId::Sftp, QStringLiteral("Remote filesystem browser"));
    pluginNames.insert(PluginId::ClipBoard, QStringLiteral("Clipboard"));
    pluginNames.insert(PluginId::SystemVolume, QStringLiteral("System volume"));
    pluginNames.insert(PluginId::Presenter, QStringLiteral("Presenter"));
    pluginNames.insert(PluginId::VirtualInput, QStringLiteral("Virtual input"));

    return pluginNames;
}

QString pluginIdString(PluginId id)
{
    static QHash<PluginId, QString> pluginNames = initPluginNameHashTable();
    return pluginNames[id];
}
