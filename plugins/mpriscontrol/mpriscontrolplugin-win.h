/**
 * SPDX-FileCopyrightText: 2018 Jun Bo Bi <jambonmcyeah@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "core/plugins/kdeconnectplugin.h"
#include "mediaplaybackcontroller.h"

#define PACKET_TYPE_MPRIS QStringLiteral("kdeconnect.mpris")

class MprisControlPlugin : public KdeConnectPlugin
{
    Q_OBJECT

public:
    explicit MprisControlPlugin(QObject *parent, const QVariantList &args);

protected Q_SLOTS:
    void sendPlayerList();
    void sendPlaybackInfo(QString playerName);

protected:
    virtual void receivePacket(const NetworkPacket &np) override;
    virtual void onPluginEnabled() override;

    MediaPlayBackController *m_mediaPlaybackController;
};
