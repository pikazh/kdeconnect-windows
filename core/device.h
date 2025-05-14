/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef DEVICE_H
#define DEVICE_H

#include <QEnableSharedFromThis>
#include <QHostAddress>
#include <QObject>
#include <QSettings>
#include <QString>
#include <memory>

#include "QObjectPtr.h"
#include "backends/devicelink.h"
#include "deviceinfo.h"
#include "networkpacket.h"

class DeviceLink;
class KdeConnectPlugin;

class KDECONNECTCORE_EXPORT Device : public QObject, public QEnableSharedFromThis<Device>
{
    friend class DeviceManager;
    Q_OBJECT
    Q_PROPERTY(QString type READ typeAsString NOTIFY typeChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString verificationKey READ verificationKey CONSTANT)
    Q_PROPERTY(bool isReachable READ isReachable NOTIFY reachableChanged)
    Q_PROPERTY(bool isPaired READ isPaired NOTIFY pairStateChanged)
    Q_PROPERTY(bool isPairRequested READ isPairRequested NOTIFY pairStateChanged)
    Q_PROPERTY(bool isPairRequestedByPeer READ isPairRequestedByPeer NOTIFY pairStateChanged)
    Q_PROPERTY(PairState pairState READ pairState NOTIFY pairStateChanged)

public:
    using Ptr = shared_qobject_ptr<Device>;

    enum class PairState {
        NotPaired,
        Requested,
        RequestedByPeer,
        Paired,
    };
    Q_ENUM(PairState)

    Device(QObject *parent, const DeviceInfo &deviceInfo, bool isPaired);
    virtual ~Device() override;

    QString id() const;
    QString name() const;

    DeviceType type() const;
    QString typeAsString() const { return type().toString(); }

    QSslCertificate certificate() const;
    int protocolVersion() const;

    Q_SCRIPTABLE QString verificationKey() const;
    Q_SCRIPTABLE QString encryptionInfo() const;

    PairState pairState() const;

    Q_SCRIPTABLE bool isPaired() const;
    Q_SCRIPTABLE bool isPairRequested() const;
    Q_SCRIPTABLE bool isPairRequestedByPeer() const;
    virtual bool isReachable() const;

    Q_SCRIPTABLE QStringList loadedPlugins() const;
    Q_SCRIPTABLE bool hasPlugin(const QString &pluginId) const;

    KdeConnectPlugin *plugin(const QString &pluginId) const;
    Q_SCRIPTABLE void setPluginEnabled(const QString &pluginId, bool enabled);
    Q_SCRIPTABLE bool isPluginEnabled(const QString &pluginId);

    QStringList supportedPlugins() const;

    QHostAddress getLocalIpAddress() const;

protected:
    // Add and remove links
    void addLink(DeviceLink *link);
    void removeLink(DeviceLink *link);

    bool updateDeviceInfo(const DeviceInfo &deviceInfo);

    QString pluginsConfigFile() const;
    QSettings *settingConfig();

public Q_SLOTS:
    /// sends a @p np packet to the device
    /// virtual for testing purposes.
    virtual bool sendPacket(NetworkPacket &np);

public Q_SLOTS:
    Q_SCRIPTABLE void requestPairing();
    Q_SCRIPTABLE void unpair();
    Q_SCRIPTABLE void reloadPlugins(); // from kconf

    Q_SCRIPTABLE void acceptPairing();
    Q_SCRIPTABLE void cancelPairing();

private Q_SLOTS:
    void privateReceivedPacket(const NetworkPacket &np);
    void linkDestroyed(QObject *o);

    void pairingHandler_incomingPairRequest();
    void pairingHandler_pairingFailed(const QString &errorMessage);
    void pairingHandler_pairingSuccessful();
    void pairingHandler_unpaired();

Q_SIGNALS:
    Q_SCRIPTABLE void pluginsChanged();
    Q_SCRIPTABLE void reachableChanged(bool reachable);
    Q_SCRIPTABLE void pairStateChanged(PairState pairState);
    Q_SCRIPTABLE void pairingFailed(const QString &error);
    Q_SCRIPTABLE void nameChanged(const QString &name);
    Q_SCRIPTABLE void typeChanged(const QString &type);

private:
    struct DevicePrivate;
    std::unique_ptr<DevicePrivate> d;
};

Q_DECLARE_METATYPE(Device*)

#endif // DEVICE_H
