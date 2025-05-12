/**
 * SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "core/plugins/kdeconnectplugin.h"

#include <QObject>
#include <QPoint>
#include <QVariantMap>

#define PACKET_TYPE_MOUSEPAD_REQUEST QStringLiteral("kdeconnect.mousepad.request")

class RemoteControlPlugin : public KdeConnectPlugin
{
    Q_OBJECT

public:
    using KdeConnectPlugin::KdeConnectPlugin;

public Q_SLOTS:
    void moveCursor(const QPoint &p);
    void sendCommand(const QVariantMap &body);

    void sendSingleClick();
    void sendMiddleClick();
    void sendRightClick();
    void sendSingleHold();
    void sendSingleRelease();
    void sendScroll(int dx, int dy);
};
