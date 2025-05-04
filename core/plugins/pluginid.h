#pragma once

#include "kdeconnectcore_export.h"

#include <QString>

enum class PluginId {
    Ping = 0,
    BatteryMonitor,
    MprisRemote,
};

QString KDECONNECTCORE_EXPORT pluginIdString(PluginId name);
