/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>

#include "core/kdeconnectplugin.h"

#define PACKET_TYPE_PING QStringLiteral("kdeconnect.ping")
#define PINGPLUGIN_IID "org.kde.kdeconnect.device.ping"

class PingPlugin : public KdeConnectPlugin
{
    Q_OBJECT

    Q_INTERFACES(KdeConnectPlugin)
    Q_PLUGIN_METADATA(IID PINGPLUGIN_IID FILE "kdeconnect_ping.json")

public:
    PingPlugin(QObject *parent= nullptr);

    Q_SCRIPTABLE void sendPing();
    Q_SCRIPTABLE void sendPing(const QString &customMessage);

    void receivePacket(const NetworkPacket &np) override;
    //QString dbusPath() const override;
};

