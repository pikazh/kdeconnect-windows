/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef KDECONNECTPLUGIN_H
#define KDECONNECTPLUGIN_H

#include <QObject>
#include <QVariantList>

#include "device.h"
#include "kdeconnectcore_export.h"
#include "kdeconnectpluginconfig.h"
#include "networkpacket.h"

struct KdeConnectPluginPrivate;

class KDECONNECTCORE_EXPORT KdeConnectPlugin : public QObject
{
    friend class Device;
    Q_OBJECT
public:
    KdeConnectPlugin(QObject *parent, const QVariantList &args);
    virtual ~KdeConnectPlugin() override;

    const Device *device();
    Device const *device() const;

    bool sendPacket(NetworkPacket &np) const;

    KdeConnectPluginConfig *config() const;

    QString pluginId() const;

    bool isEnabled() { return m_isEnabled; }

protected:
    /**
     * Returns true if it has handled the packet in some way
     * device.sendPacket can be used to send an answer back to the device
     */
    virtual void receivePacket(const NetworkPacket &np);

    virtual void onPluginEnabled() {}
    virtual void onPluginDisabled() {}

    void enable()
    {
        m_isEnabled = true;
        onPluginEnabled();
    }

    void disable()
    {
        m_isEnabled = false;
        onPluginDisabled();
    }

private:
    const std::unique_ptr<KdeConnectPluginPrivate> d;
    bool m_isEnabled = false;
};

#endif
