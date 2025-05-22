#pragma once

#include "kdeconnectcore_export.h"

#include <QString>

enum class PluginId {
    Ping = 0,
    FindMyPhone,
    BatteryMonitor,
    MprisRemote,
    MprisController,
    Sftp,
    ClipBoard,
    SystemVolume,
    Presenter,
    VirtualInput,
    RemoteMousePad,
    RemoteKeyboard,
    Telephony,
    Sms,
    Contacts,
};

QString KDECONNECTCORE_EXPORT pluginIdString(PluginId name);
