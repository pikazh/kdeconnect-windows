#include "pluginid.h"

#include <QHash>

QHash<PluginId, QString> initPluginNameHashTable()
{
    QHash<PluginId, QString> pluginNames;
    pluginNames.insert(PluginId::Ping, QLatin1StringView("Ping"));
    pluginNames.insert(PluginId::FindMyPhone, QLatin1StringView("Ring my phone"));
    pluginNames.insert(PluginId::BatteryMonitor, QLatin1StringView("Battery monitor"));
    pluginNames.insert(PluginId::MprisRemote, QLatin1StringView("MprisRemote"));
    pluginNames.insert(PluginId::MprisController, QLatin1StringView("Multimedia control receiver"));
    pluginNames.insert(PluginId::Sftp, QLatin1StringView("Remote filesystem browser"));
    pluginNames.insert(PluginId::ClipBoard, QLatin1StringView("Clipboard"));
    pluginNames.insert(PluginId::SystemVolume, QLatin1StringView("System volume"));
    pluginNames.insert(PluginId::RemoteSystemVolume, QLatin1StringView("Remote system volume"));
    pluginNames.insert(PluginId::Presenter, QLatin1StringView("Presenter"));
    pluginNames.insert(PluginId::VirtualInput, QLatin1StringView("Virtual input"));
    pluginNames.insert(PluginId::RemoteMousePad, QLatin1StringView("RemoteControl"));
    pluginNames.insert(PluginId::RemoteKeyboard,
                       QLatin1StringView("Remote keyboard from the desktop"));
    pluginNames.insert(PluginId::Telephony, QLatin1StringView("Telephony integration"));
    pluginNames.insert(PluginId::Sms, QLatin1StringView("SMS"));
    pluginNames.insert(PluginId::Contacts, QLatin1StringView("Contacts"));
    pluginNames.insert(PluginId::RunCommand, QLatin1StringView("Run commands"));
    pluginNames.insert(PluginId::RemoteCommands, QLatin1StringView("Host remote commands"));
    pluginNames.insert(PluginId::Share, QLatin1StringView("Share and receive"));

    return pluginNames;
}

QString pluginIdString(PluginId id)
{
    static QHash<PluginId, QString> pluginNames = initPluginNameHashTable();
    return pluginNames[id];
}
