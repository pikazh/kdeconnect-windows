#include "pluginid.h"

#include <QHash>

QHash<PluginId, QString> initPluginNameHashTable()
{
    QHash<PluginId, QString> pluginNames;
    pluginNames.insert(PluginId::Ping, QStringLiteral("Ping"));
    pluginNames.insert(PluginId::BatteryMonitor, QStringLiteral("Battery monitor"));
    pluginNames.insert(PluginId::MprisRemote, QStringLiteral("MprisRemote"));
    return pluginNames;
}

QString pluginIdString(PluginId name)
{
    static QHash<PluginId, QString> pluginNames = initPluginNameHashTable();
    return pluginNames[name];
}
