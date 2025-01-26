/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "pingplugin.h"

//#include <core/daemon.h>
//#include <core/device.h>

#include "plugin_ping_debug.h"

//K_PLUGIN_CLASS_WITH_JSON(PingPlugin, "kdeconnect_ping.json")

void PingPlugin::receivePacket(const NetworkPacket &np)
{
    // Daemon::instance()->sendSimpleNotification(QStringLiteral("pingReceived"),
    //                                            device()->name(),
    //                                            np.get<QString>(QStringLiteral("message"), i18n("Ping!")),
    //                                            QStringLiteral("dialog-ok"));
}



PingPlugin::PingPlugin(QObject *parent)
    : KdeConnectPlugin(parent)
{

}

void PingPlugin::sendPing()
{
    NetworkPacket np(PACKET_TYPE_PING);
    bool success = sendPacket(np);
    qCDebug(KDECONNECT_PLUGIN_PING) << "sendPing:" << success;
}

void PingPlugin::sendPing(const QString &customMessage)
{
    NetworkPacket np(PACKET_TYPE_PING);
    if (!customMessage.isEmpty()) {
        np.set(QStringLiteral("message"), customMessage);
    }
    bool success = sendPacket(np);
    qCDebug(KDECONNECT_PLUGIN_PING) << "sendPing:" << success;
}
