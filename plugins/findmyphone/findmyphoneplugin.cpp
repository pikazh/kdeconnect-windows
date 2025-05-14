/**
 * SPDX-FileCopyrightText: 2014 Apoorv Parle <apparle@gmail.com>
 * SPDX-FileCopyrightText: 2015 David Edmundson <davidedmundson@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "findmyphoneplugin.h"
#include "core/plugins/pluginfactory.h"

K_PLUGIN_CLASS_WITH_JSON(FindMyPhonePlugin, "kdeconnect_findmyphone.json")

void FindMyPhonePlugin::ring()
{
    NetworkPacket np(PACKET_TYPE_FINDMYPHONE_REQUEST);
    sendPacket(np);
}

#include "findmyphoneplugin.moc"
