/**
 * SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "remotecontrolplugin.h"
#include "core/plugins/pluginfactory.h"

//#include "plugin_remotecontrol_debug.h"

K_PLUGIN_CLASS_WITH_JSON(RemoteControlPlugin, "kdeconnect_remotecontrol.json")

void RemoteControlPlugin::moveCursor(const QPoint &p)
{
    NetworkPacket np(PACKET_TYPE_MOUSEPAD_REQUEST, {{QStringLiteral("dx"), p.x()}, {QStringLiteral("dy"), p.y()}});
    sendPacket(np);
}

void RemoteControlPlugin::sendCommand(const QVariantMap &body)
{
    if (body.isEmpty())
        return;
    NetworkPacket np(PACKET_TYPE_MOUSEPAD_REQUEST, body);
    sendPacket(np);
}

void RemoteControlPlugin::sendSingleClick()
{
    sendCommand({{QStringLiteral("singleclick"), true}});
}

void RemoteControlPlugin::sendMiddleClick()
{
    sendCommand({{QStringLiteral("middleclick"), true}});
}

void RemoteControlPlugin::sendRightClick()
{
    sendCommand({{QStringLiteral("rightclick"), true}});
}

void RemoteControlPlugin::sendSingleHold()
{
    sendCommand({{QStringLiteral("singlehold"), true}});
}

void RemoteControlPlugin::sendSingleRelease()
{
    sendCommand({{QStringLiteral("singlerelease"), true}});
}

void RemoteControlPlugin::sendScroll(int dx, int dy)
{
    QVariantMap body;
    body[QStringLiteral("scroll")] = true;
    body[QStringLiteral("dx")] = dx;
    body[QStringLiteral("dy")] = dy;
    sendCommand(body);
}

#include "remotecontrolplugin.moc"
