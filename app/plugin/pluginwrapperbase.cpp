#include "pluginwrapperbase.h"
#include <QObject>

PluginWrapperBase::PluginWrapperBase(Device::Ptr device, PluginId pluginId, QObject *parent)
    : QObject(parent)
    , m_deviceWeakPtr(device)
    , m_sourcePluginId(pluginId)
{
    QObject::connect(device.get(), SIGNAL(pluginsChanged()), this, SLOT(reloadPlugin()));
}

void PluginWrapperBase::init()
{
    m_sourcePlugin = getPlugin(m_sourcePluginId);
}

KdeConnectPlugin *PluginWrapperBase::getPlugin(PluginId pluginId)
{
    auto devicePtr = m_deviceWeakPtr.lock();
    if (!devicePtr) {
        return nullptr;
    }

    auto plugin = devicePtr->plugin(pluginIdString(pluginId));
    if (plugin != nullptr) {
        this->connectPluginSignals(plugin);
    }

    return plugin;
}

void PluginWrapperBase::reloadPlugin()
{
    auto plugin = getPlugin(m_sourcePluginId);
    if (m_sourcePlugin != plugin) {
        m_sourcePlugin = plugin;

        emit pluginLoadedChange(m_sourcePlugin != nullptr);
    }
}
