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
    static PluginLoader *instance = new PluginLoader();
    return instance;
}

PluginLoader::PluginLoader()
{
    const QList<PluginMetaData> data = PluginMetaData::load();
    for (const PluginMetaData &metadata : data)
    {
        plugins[metadata.pluginId()] = metadata;
    }
}

QStringList PluginLoader::getPluginList() const
{
    return plugins.keys();
}

bool PluginLoader::doesPluginExist(const QString &name) const
{
    return plugins.contains(name);
}

PluginMetaData PluginLoader::getPluginInfo(const QString &name) const
{
    return plugins.value(name);
}

KdeConnectPlugin *PluginLoader::instantiatePluginForDevice(const QString &pluginName, Device *device) const
{
    const PluginMetaData data = plugins.value(pluginName);
    if (!data.isValid())
    {
        qWarning(KDECONNECT_CORE) << "Plugin unknown " << pluginName;
        return nullptr;
    }

    const QStringList outgoingInterfaces = data.value(QStringLiteral("X-KdeConnect-OutgoingPacketType"), QStringList());
    const QVariantList args{QVariant::fromValue<Device *>(device), pluginName, outgoingInterfaces, data.iconName()};

    KdeConnectPlugin* plugin = PluginFactory::instantiatePlugin<KdeConnectPlugin>(data, device, args);
    if(plugin != nullptr)
    {
        qInfo(KDECONNECT_CORE) << "Loaded plugin:" << data.pluginId();
    }
    else
    {
        qWarning(KDECONNECT_CORE) << "Error loading plugin:" << data.pluginId();
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
    QString myDeviceType = KdeConnectConfig::instance().deviceType().toString();

    for (const PluginMetaData &service : plugins)
    {
        // Check if the plugin support this device type
        const QStringList supportedDeviceTypes = service.value(QStringLiteral("X-KdeConnect-SupportedDeviceTypes"), QStringList());
        if (!supportedDeviceTypes.isEmpty())
        {
            if (!supportedDeviceTypes.contains(myDeviceType))
            {
                qWarning(KDECONNECT_CORE) << "Not loading plugin " << service.pluginId() << ", because this device of type " << myDeviceType
                                         << " is not supported. Supports: " << supportedDeviceTypes.join(QStringLiteral(", "));
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
                qInfo(KDECONNECT_CORE) << "Not loading plugin " << service.pluginId() << ", because device doesn't support it";
                continue;
            }
        }

        // If we get here, the plugin can be loaded
        ret += service.pluginId();
    }

    return ret;
}
