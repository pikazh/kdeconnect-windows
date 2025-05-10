/**
 * SPDX-FileCopyrightText: 2018 Jun Bo Bi <jambonmcyeah@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "core/plugins/kdeconnectplugin.h"

#include <QMap>
#include <QObject>

#include <Windows.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>

#define PACKET_TYPE_SYSTEMVOLUME QStringLiteral("kdeconnect.systemvolume")
#define PACKET_TYPE_SYSTEMVOLUME_REQUEST QStringLiteral("kdeconnect.systemvolume.request")

class SystemvolumePlugin : public KdeConnectPlugin
{
    Q_OBJECT

public:
    explicit SystemvolumePlugin(QObject *parent, const QVariantList &args);
    virtual ~SystemvolumePlugin() override;

protected:
    virtual void receivePacket(const NetworkPacket &np) override;
    virtual void onPluginEnabled() override;

    bool sendSinkList();

    HRESULT setDefaultAudioPlaybackDevice(QString &name, bool enabled);

private:
    class CMMNotificationClient;
    class CAudioEndpointVolumeCallback;

    bool valid = false;
    IMMDeviceEnumerator *deviceEnumerator;
    CMMNotificationClient *deviceCallback;
    QMap<QString, QPair<IAudioEndpointVolume *, CAudioEndpointVolumeCallback *>> sinkList;
    QMap<QString, QString> idToNameMap;
};
