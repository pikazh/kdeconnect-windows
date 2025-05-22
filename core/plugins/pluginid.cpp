#include "pluginid.h"

#include <QHash>

QHash<PluginId, QString> initPluginNameHashTable()
{
    QHash<PluginId, QString> pluginNames;
    pluginNames.insert(PluginId::Ping, QStringLiteral("Ping"));
    pluginNames.insert(PluginId::FindMyPhone, QStringLiteral("Ring my phone"));
    pluginNames.insert(PluginId::BatteryMonitor, QStringLiteral("Battery monitor"));
    pluginNames.insert(PluginId::MprisRemote, QStringLiteral("MprisRemote"));
    pluginNames.insert(PluginId::MprisController, QStringLiteral("Multimedia control receiver"));
    pluginNames.insert(PluginId::Sftp, QStringLiteral("Remote filesystem browser"));
    pluginNames.insert(PluginId::ClipBoard, QStringLiteral("Clipboard"));
    pluginNames.insert(PluginId::SystemVolume, QStringLiteral("System volume"));
    pluginNames.insert(PluginId::Presenter, QStringLiteral("Presenter"));
    pluginNames.insert(PluginId::VirtualInput, QStringLiteral("Virtual input"));
    pluginNames.insert(PluginId::RemoteMousePad, QStringLiteral("RemoteControl"));
    pluginNames.insert(PluginId::RemoteKeyboard, QStringLiteral("Remote keyboard from the desktop"));
    pluginNames.insert(PluginId::Telephony, QStringLiteral("Telephony integration"));
    pluginNames.insert(PluginId::Sms, QStringLiteral("SMS"));
    pluginNames.insert(PluginId::Contacts, QStringLiteral("Contacts"));
    return pluginNames;
}

QString pluginIdString(PluginId id)
{
    static QHash<PluginId, QString> pluginNames = initPluginNameHashTable();
    return pluginNames[id];
}
