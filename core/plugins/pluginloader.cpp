#include "pluginloader.h"
#include "core_debug.h"
#include "device.h"
#include "kdeconnectconfig.h"
#include "kdeconnectplugin.h"
#include "pluginfactory.h"

#include <QPluginLoader>
#include <QList>

PluginLoader *PluginLoader::instance()
{
    static PluginLoader instance;
    return &instance;
}

PluginLoader::PluginLoader()
{
    const QList<PluginMetaData> data = PluginMetaData::load();
    for (const PluginMetaData &metadata : data)
    {
        plugins[metadata.id()] = metadata;
    }
}

QStringList PluginLoader::getPluginList() const
{
    return plugins.keys();
}

bool PluginLoader::doesPluginExist(const QString &id) const
{
    return plugins.contains(id);
}

PluginMetaData PluginLoader::getPluginInfo(const QString &id) const
{
    return plugins.value(id);
}

KdeConnectPlugin *PluginLoader::instantiatePluginForDevice(const QString &pluginId,
                                                           Device *device) const
{
    const PluginMetaData data = plugins.value(pluginId);
    if (!data.isValid())
    {
        qWarning(KDECONNECT_CORE) << "Plugin unknown:" << pluginId;
        return nullptr;
    }

    const QStringList outgoingInterfaces = data.value(QStringLiteral("X-KdeConnect-OutgoingPacketType"), QStringList());
    const QVariantList args{QVariant::fromValue<Device *>(device), data.id(), outgoingInterfaces};

    KdeConnectPlugin* plugin = PluginFactory::instantiatePlugin<KdeConnectPlugin>(data, device, args);
    if(plugin != nullptr)
    {
        qWarning(KDECONNECT_CORE) << "Loaded plugin:" << data.id();
    }
    else
    {
        qWarning(KDECONNECT_CORE) << "Error loading plugin:" << data.id();
    }

    return plugin;
}

QStringList PluginLoader::incomingCapabilities() const
{
    QSet<QString> ret;
    for (const PluginMetaData &service : plugins)
    {
        QStringList rawValues = service.value(QStringLiteral("X-KdeConnect-SupportedPacketType"), QStringList());
        ret += QSet<QString>(rawValues.begin(), rawValues.end());
    }
    return ret.values();
}

QStringList PluginLoader::outgoingCapabilities() const
{
    QSet<QString> ret;
    for (const PluginMetaData &service : plugins)
    {
        QStringList rawValues = service.value(QStringLiteral("X-KdeConnect-OutgoingPacketType"), QStringList());
        ret += QSet<QString>(rawValues.begin(), rawValues.end());
    }
    return ret.values();
}

QSet<QString> PluginLoader::pluginsForCapabilities(const QSet<QString> &incoming, const QSet<QString> &outgoing) const
{
    QSet<QString> ret;
    QString myDeviceType = KdeConnectConfig::instance()->deviceType().toString();

    for (const PluginMetaData &service : plugins)
    {
        // Check if the plugin support this device type
        const QStringList supportedDeviceTypes = service.value(QStringLiteral("X-KdeConnect-SupportedDeviceTypes"), QStringList());
        if (!supportedDeviceTypes.isEmpty())
        {
            if (!supportedDeviceTypes.contains(myDeviceType))
            {
                qWarning(KDECONNECT_CORE)
                    << "Not loading plugin " << service.id() << ", because this device of type "
                    << myDeviceType << " is not supported. Supports: "
                    << supportedDeviceTypes.join(QStringLiteral(", "));
                continue;
            }
        }

        // Check if capbilites intersect with the remote device
        const QStringList pluginIncomingCapabilities = service.value(QStringLiteral("X-KdeConnect-SupportedPacketType"), QStringList());
        const QStringList pluginOutgoingCapabilities = service.value(QStringLiteral("X-KdeConnect-OutgoingPacketType"), QStringList());

        bool capabilitiesEmpty = (pluginIncomingCapabilities.isEmpty() && pluginOutgoingCapabilities.isEmpty());
        if (!capabilitiesEmpty)
        {
            bool capabilitiesIntersect = (outgoing.intersects(QSet(pluginIncomingCapabilities.begin(), pluginIncomingCapabilities.end()))
                                          || incoming.intersects(QSet(pluginOutgoingCapabilities.begin(), pluginOutgoingCapabilities.end())));

            if (!capabilitiesIntersect)
            {
                qWarning(KDECONNECT_CORE) << "Not loading plugin " << service.id()
                                          << ", because peer device doesn't support it";
                continue;
            }
        }

        // If we get here, the plugin can be loaded
        ret += service.id();
    }

    return ret;
}
