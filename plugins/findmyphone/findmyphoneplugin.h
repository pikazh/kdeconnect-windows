/**
 * SPDX-FileCopyrightText: 2014 Apoorv Parle <apparle@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>

#include "core/plugins/kdeconnectplugin.h"

#define PACKET_TYPE_FINDMYPHONE_REQUEST QStringLiteral("kdeconnect.findmyphone.request")

class FindMyPhonePlugin : public KdeConnectPlugin
{
    Q_OBJECT

public:
    using KdeConnectPlugin::KdeConnectPlugin;

public Q_SLOTS:
    void ring();
};
