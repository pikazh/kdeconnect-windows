/**
 * SPDX-FileCopyrightText: 2018 Nicolas Fella <nicolas.fella@gmx.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>

#include "core/plugins/kdeconnectplugin.h"

#define PACKET_TYPE_SYSTEMVOLUME QStringLiteral("kdeconnect.systemvolume")
#define PACKET_TYPE_SYSTEMVOLUME_REQUEST QStringLiteral("kdeconnect.systemvolume.request")

class RemoteSystemVolumePlugin : public KdeConnectPlugin
{
    Q_OBJECT
    Q_PROPERTY(QByteArray sinks READ sinks NOTIFY sinksChanged)

public:
    using KdeConnectPlugin::KdeConnectPlugin;

    QByteArray sinks();

    Q_SCRIPTABLE void sendVolume(const QString &name, int volume);
    Q_SCRIPTABLE void sendMuted(const QString &name, bool muted);

protected:
    virtual void receivePacket(const NetworkPacket &np) override;
    virtual void onPluginEnabled() override;

Q_SIGNALS:
    Q_SCRIPTABLE void sinksChanged();
    Q_SCRIPTABLE void volumeChanged(const QString &name, int volume);
    Q_SCRIPTABLE void mutedChanged(const QString &name, bool muted);

private:
    QByteArray m_sinks;
};
