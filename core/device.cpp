#include "device.h"
#include "backends/devicelink.h"
#include "backends/lan/landevicelink.h"
#include "backends/linkprovider.h"
#include "core_debug.h"
#include "kdeconnectconfig.h"
#include "networkpacket.h"
#include "plugins/kdeconnectplugin.h"
#include "plugins/pluginloader.h"

#include <QSet>
#include <QSettings>
#include <QSslCertificate>
#include <QSslKey>
#include <QVector>

struct Device::DevicePrivate
{
    DevicePrivate(const DeviceInfo &deviceInfo)
        : m_deviceInfo(deviceInfo)
    {
    }

    DeviceInfo m_deviceInfo;

    QVector<DeviceLink *> m_deviceLinks;
    QHash<QString, KdeConnectPlugin *> m_plugins;

    QMultiMap<QString, KdeConnectPlugin *> m_pluginsByIncomingCapability;
    QSet<QString> m_supportedPlugins;
    PairingHandler *m_pairingHandler = nullptr;
    QSettings *m_settings = nullptr;
};

Device::Device(QObject *parent, const DeviceInfo &deviceInfo, bool isPaired)
    : QObject(parent)
    , d(new Device::DevicePrivate(deviceInfo))
{
    d->m_pairingHandler = new PairingHandler(this,
                                             isPaired ? PairState::Paired : PairState::NotPaired);

    const auto supported = PluginLoader::instance()->getPluginList();
    // Assume every plugin is supported until we get the capabilities
    d->m_supportedPlugins = QSet(supported.begin(), supported.end());

    connect(d->m_pairingHandler,
            &PairingHandler::incomingPairRequest,
            this,
            &Device::pairingHandler_incomingPairRequest);
    connect(d->m_pairingHandler,
            &PairingHandler::pairingFailed,
            this,
            &Device::pairingHandler_pairingFailed);
    connect(d->m_pairingHandler,
            &PairingHandler::pairingSuccessful,
            this,
            &Device::pairingHandler_pairingSuccessful);
    connect(d->m_pairingHandler, &PairingHandler::unpaired, this, &Device::pairingHandler_unpaired);
}

Device::~Device()
{
    for (auto p : d->m_plugins) {
        p->disable();
    }
    qDeleteAll(d->m_plugins);
    qDeleteAll(d->m_deviceLinks);
}

QString Device::id() const
{
    return d->m_deviceInfo.id;
}

QString Device::name() const
{
    return d->m_deviceInfo.name;
}

DeviceType Device::type() const
{
    return d->m_deviceInfo.type;
}

bool Device::isReachable() const
{
    return !d->m_deviceLinks.isEmpty();
}

QStringList Device::supportedPlugins() const
{
    return QList(d->m_supportedPlugins.cbegin(), d->m_supportedPlugins.cend());
}

bool Device::hasPlugin(const QString &pluginId) const
{
    return d->m_plugins.contains(pluginId);
}

QStringList Device::loadedPlugins() const
{
    return d->m_plugins.keys();
}

void Device::reloadPlugins()
{
    qCDebug(KDECONNECT_CORE) << name() << "- reload plugins";

    QHash<QString, KdeConnectPlugin *> newPluginMap, oldPluginMap = d->m_plugins;
    QMultiMap<QString, KdeConnectPlugin *> newPluginsByIncomingCapability;

    if (isPaired() && isReachable()) { // Do not load any plugin for unpaired devices, nor useless loading them for unreachable devices

        PluginLoader *loader = PluginLoader::instance();

        for (const QString &pluginId : std::as_const(d->m_supportedPlugins)) {
            const PluginMetaData service = loader->getPluginInfo(pluginId);

            const bool pluginEnabled = isPluginEnabled(pluginId);
            if (pluginEnabled) {
                KdeConnectPlugin *plugin = d->m_plugins.take(pluginId);

                if (plugin == nullptr) {
                    qCDebug(KDECONNECT_CORE) << "loading plugin:" << pluginId;
                    plugin = loader->instantiatePluginForDevice(pluginId, this);
                    plugin->enable();
                } else {
                    qCDebug(KDECONNECT_CORE) << "plugin:" << pluginId << "is loaded.";
                }
                Q_ASSERT(plugin);

                const QStringList incomingCapabilities
                    = service.rawData()
                          .value(QStringLiteral("X-KdeConnect-SupportedPacketType"))
                          .toVariant()
                          .toStringList();

                for (const QString &interface : incomingCapabilities) {
                    newPluginsByIncomingCapability.insert(interface, plugin);
                }

                newPluginMap[pluginId] = plugin;
            }
        }
    }

    const bool differentPlugins = oldPluginMap != newPluginMap;

    // Erase all left plugins in the original map (meaning that we don't want
    // them anymore, otherwise they would have been moved to the newPluginMap)
    for (auto p : d->m_plugins) {
        p->disable();
    }
    qDeleteAll(d->m_plugins);
    d->m_plugins = newPluginMap;
    d->m_pluginsByIncomingCapability = newPluginsByIncomingCapability;

    if (differentPlugins)
    {
        Q_EMIT pluginsChanged();
    }
}

QString Device::pluginsConfigFile() const
{
    return KdeConnectConfig::instance()->deviceConfigDir(id()).absoluteFilePath(QStringLiteral("config"));
}

void Device::requestPairing()
{
    qCDebug(KDECONNECT_CORE) << "Request pairing";
    d->m_pairingHandler->requestPairing();
    Q_EMIT pairStateChanged(pairState());
}

void Device::unpair()
{
    qCDebug(KDECONNECT_CORE) << "Request unpairing";
    d->m_pairingHandler->unpair();
}

void Device::acceptPairing()
{
    qCDebug(KDECONNECT_CORE) << "Accept pairing";
    d->m_pairingHandler->acceptPairing();
}

void Device::cancelPairing()
{
    qCDebug(KDECONNECT_CORE) << "Cancel pairing";
    d->m_pairingHandler->cancelPairing();
}

void Device::pairingHandler_incomingPairRequest()
{
    Q_ASSERT(d->m_pairingHandler->pairState() == PairState::RequestedByPeer);
    Q_EMIT pairStateChanged(pairState());
}

void Device::pairingHandler_pairingSuccessful()
{
    Q_ASSERT(d->m_pairingHandler->pairState() == PairState::Paired);
    KdeConnectConfig::instance()->addTrustedDevice(d->m_deviceInfo);
    reloadPlugins(); // Will load/unload plugins
    Q_EMIT pairStateChanged(pairState());
}

void Device::pairingHandler_pairingFailed(const QString &errorMessage)
{
    Q_ASSERT(d->m_pairingHandler->pairState() == PairState::NotPaired);
    Q_EMIT pairingFailed(errorMessage);
    Q_EMIT pairStateChanged(pairState());
}

void Device::pairingHandler_unpaired()
{
    Q_ASSERT(d->m_pairingHandler->pairState() == PairState::NotPaired);
    qCDebug(KDECONNECT_CORE) << "Unpaired";
    KdeConnectConfig::instance()->removeTrustedDevice(id());
    reloadPlugins(); // Will load/unload plugins
    Q_EMIT pairStateChanged(pairState());
}

void Device::addLink(DeviceLink *link)
{
    if (d->m_deviceLinks.contains(link))
    {
        return;
    }

    d->m_deviceLinks.append(link);

    std::sort(d->m_deviceLinks.begin(), d->m_deviceLinks.end(), [](DeviceLink *a, DeviceLink *b) {
        return a->priority() > b->priority();
    });

    connect(link, &QObject::destroyed, this, &Device::linkDestroyed);
    connect(link, &DeviceLink::receivedPacket, this, &Device::privateReceivedPacket);

    bool hasChanges = updateDeviceInfo(link->deviceInfo());

    if (d->m_deviceLinks.size() == 1)
    {
        Q_EMIT reachableChanged(true);
        hasChanges = true;
    }

    if (hasChanges)
    {
        reloadPlugins();
    }
}

bool Device::updateDeviceInfo(const DeviceInfo &newDeviceInfo)
{
    bool hasChanges = false;
    if (d->m_deviceInfo.name != newDeviceInfo.name || d->m_deviceInfo.type != newDeviceInfo.type
        || d->m_deviceInfo.protocolVersion != newDeviceInfo.protocolVersion) {
        hasChanges = true;
        d->m_deviceInfo.name = newDeviceInfo.name;
        d->m_deviceInfo.type = newDeviceInfo.type;
        d->m_deviceInfo.protocolVersion = newDeviceInfo.protocolVersion;
        Q_EMIT typeChanged(d->m_deviceInfo.type.toString());
        Q_EMIT nameChanged(d->m_deviceInfo.name);
        if (isPaired())
        {
            KdeConnectConfig::instance()->updateTrustedDeviceInfo(d->m_deviceInfo);
        }
    }

    if (d->m_deviceInfo.outgoingCapabilities != newDeviceInfo.outgoingCapabilities
        || d->m_deviceInfo.incomingCapabilities != newDeviceInfo.incomingCapabilities)
    {
        if (!newDeviceInfo.incomingCapabilities.isEmpty() && !newDeviceInfo.outgoingCapabilities.isEmpty())
        {
            hasChanges = true;
            d->m_supportedPlugins = PluginLoader::instance()->pluginsForCapabilities(newDeviceInfo.incomingCapabilities, newDeviceInfo.outgoingCapabilities);
            qInfo(KDECONNECT_CORE) << "new capabilities for" << name();
        }
    }

    return hasChanges;
}

QSettings *Device::settingConfig()
{
    if (isPaired()) {
        if (d->m_settings == nullptr) {
            d->m_settings = new QSettings(pluginsConfigFile(), QSettings::IniFormat, this);
        }
    }

    return d->m_settings;
}

void Device::linkDestroyed(QObject *o)
{
    removeLink(static_cast<DeviceLink *>(o));
}

void Device::removeLink(DeviceLink *link)
{
    d->m_deviceLinks.removeAll(link);

    qCDebug(KDECONNECT_CORE) << name() << "remove Link," << d->m_deviceLinks.size()
                             << "links remaining";

    if (d->m_deviceLinks.isEmpty())
    {
        reloadPlugins();
        Q_EMIT reachableChanged(false);
    }
}

bool Device::sendPacket(NetworkPacket &np)
{
    Q_ASSERT(isPaired() || np.type() == PACKET_TYPE_PAIR);

    // Maybe we could block here any packet that is not an identity or a pairing packet to prevent sending non encrypted data
    for (DeviceLink *dl : std::as_const(d->m_deviceLinks))
    {
        if (dl->sendPacket(np))
            return true;
    }

    return false;
}

void Device::privateReceivedPacket(const NetworkPacket &np)
{
    if (np.type() == PACKET_TYPE_PAIR)
    {
        d->m_pairingHandler->packetReceived(np);
    }
    else if (isPaired())
    {
        const QList<KdeConnectPlugin*> plugins = d->m_pluginsByIncomingCapability.values(np.type());
        if (plugins.isEmpty())
        {
            qWarning(KDECONNECT_CORE) << "discarding unsupported packet " << np.type() << " for " << name();
        }
        for (KdeConnectPlugin *plugin : plugins)
        {
            plugin->receivePacket(np);
        }
    }
    else
    {
        qWarning(KDECONNECT_CORE) << "device" << name() << "not paired, ignoring packet"
                                  << np.type();
        unpair();
    }
}

Device::PairState Device::pairState() const
{
    return d->m_pairingHandler->pairState();
}

bool Device::isPaired() const
{
    return d->m_pairingHandler->pairState() == PairState::Paired;
}

bool Device::isPairRequested() const
{
    return d->m_pairingHandler->pairState() == PairState::Requested;
}

bool Device::isPairRequestedByPeer() const
{
    return d->m_pairingHandler->pairState() == PairState::RequestedByPeer;
}

QHostAddress Device::getLocalIpAddress() const
{
    for (DeviceLink *dl : std::as_const(d->m_deviceLinks))
    {
        LanDeviceLink *ldl = dynamic_cast<LanDeviceLink *>(dl);
        if (ldl)
        {
            return ldl->hostAddress();
        }
    }
    return QHostAddress::Null;
}

KdeConnectPlugin *Device::plugin(const QString &pluginId) const
{
    auto it = d->m_plugins.constFind(pluginId);
    if (it != d->m_plugins.constEnd()) {
        return it.value();
    }

    return nullptr;
}

void Device::setPluginEnabled(const QString &pluginId, bool enabled)
{
    auto settings = settingConfig();
    if (settings != nullptr) {
        settings->beginGroup(QLatin1StringView("Plugins"));
        settings->setValue(pluginId, enabled);
        settings->endGroup();
    }
}

bool Device::isPluginEnabled(const QString &pluginId)
{
    auto settings = settingConfig();
    if (settings != nullptr) {
        settings->beginGroup(QLatin1StringView("Plugins"));
        QString val = settings->value(pluginId, QVariant()).toString();
        settings->endGroup();
        if (val.isEmpty())
            return PluginLoader::instance()->getPluginInfo(pluginId).isEnabledByDefault();
        else
            return val == QLatin1StringView("true");
    }

    return false;
}

QString Device::encryptionInfo() const
{
    QString result;
    const QCryptographicHash::Algorithm digestAlgorithm = QCryptographicHash::Algorithm::Sha256;

    QString localChecksum = QString::fromLatin1(KdeConnectConfig::instance()->certificate().digest(digestAlgorithm).toHex());
    for (int i = 2; i < localChecksum.size(); i += 3) {
        localChecksum.insert(i, QLatin1Char(':')); // Improve readability
    }
    result += QString("SHA256 fingerprint of your device certificate is: %1\n").arg(localChecksum);

    QString remoteChecksum = QString::fromLatin1(certificate().digest(digestAlgorithm).toHex());
    for (int i = 2; i < remoteChecksum.size(); i += 3) {
        remoteChecksum.insert(i, QLatin1Char(':')); // Improve readability
    }
    result += QString("SHA256 fingerprint of remote device certificate is: %1\n").arg(remoteChecksum);

    result += QString("Protocol version: %1\n").arg(d->m_deviceInfo.protocolVersion);

    return result;
}

QSslCertificate Device::certificate() const
{
    return d->m_deviceInfo.certificate;
}

int Device::protocolVersion() const
{
    return d->m_deviceInfo.protocolVersion;
}

QString Device::verificationKey() const
{
    return d->m_pairingHandler->verificationKey();
}

QString Device::iconName() const
{
    return d->m_deviceInfo.type.icon();
}

QString Device::statusIconName() const
{
    return d->m_deviceInfo.type.iconForStatus(isReachable(), isPaired());
}
