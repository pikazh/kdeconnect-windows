/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "kdeconnectplugin.h"
#include "core_debug.h"

struct KdeConnectPluginPrivate {
    Device *m_device;
    QString m_pluginId;
    QSet<QString> m_outgoingCapabilties;
    KdeConnectPluginConfig::Ptr m_config;
};

KdeConnectPlugin::KdeConnectPlugin(QObject *parent, const QVariantList &args)
    : QObject(parent)
    , d(new KdeConnectPluginPrivate)
{
    Q_ASSERT(args.length() == 3);
    d->m_device = qvariant_cast<Device *>(args.at(0));
    d->m_pluginId = args.at(1).toString();

    const QStringList cap = args.at(2).toStringList();
    d->m_outgoingCapabilties = QSet(cap.begin(), cap.end());
}

KdeConnectPlugin::~KdeConnectPlugin() {}

KdeConnectPluginConfig::Ptr KdeConnectPlugin::config() const
{
    // Create on demand, because not every plugin will use it
    if (!d->m_config) {
        d->m_config.reset(new KdeConnectPluginConfig(d->m_device->id(), d->m_pluginId));
    }
    return d->m_config;
}

QString KdeConnectPlugin::pluginId() const
{
    return d->m_pluginId;
}

const Device *KdeConnectPlugin::device()
{
    return d->m_device;
}

Device const *KdeConnectPlugin::device() const
{
    return d->m_device;
}

bool KdeConnectPlugin::sendPacket(NetworkPacket &np) const
{
    if (!d->m_outgoingCapabilties.contains(np.type())) {
        qCritical(KDECONNECT_CORE)
            << metaObject()->className() << "tried to send an unsupported packet type " << np.type()
            << ". Only Supported: " << d->m_outgoingCapabilties;
        return false;
    }
    //     qCWarning(KDECONNECT_CORE) << metaObject()->className() << "sends" << np.type() << ". Supported:" << d->mOutgoingTypes;
    return d->m_device->sendPacket(np);
}

void KdeConnectPlugin::receivePacket(const NetworkPacket &np)
{
    qCWarning(KDECONNECT_CORE) << metaObject()->className() << "received a packet of type " << np.type() << ", but doesn't implement receivePacket";
}
