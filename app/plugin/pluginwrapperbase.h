#pragma once

#include <QObject>
#include <QWeakPointer>

#include "core/device.h"
#include "core/plugins/kdeconnectplugin.h"
#include "core/plugins/pluginid.h"

class PluginWrapperBase : public QObject
{
    Q_OBJECT
public:
    explicit PluginWrapperBase(Device::Ptr device, PluginId pluginId, QObject *parent = nullptr);
    virtual ~PluginWrapperBase() override = default;

    void init();

protected:
    KdeConnectPlugin *sourcePlugin() const { return m_sourcePlugin; }

    template<typename T>
    T propertyValue(const char *name, const T &defVal = T()) const
    {
        auto plugin = sourcePlugin();
        if (plugin != nullptr) {
            return qvariant_cast<T>(plugin->property(name));
        } else {
            return defVal;
        }
    }

    template<typename T>
    void setPropertyValue(const char *name, const T &value)
    {
        auto plugin = sourcePlugin();
        if (plugin != nullptr) {
            plugin->setProperty(name, QVariant::fromValue(value));
        }
    }

    virtual void connectPluginSignals(KdeConnectPlugin *plugin) {}

private Q_SLOTS:
    void pluginsReloaded();

private:
    KdeConnectPlugin *getPlugin(PluginId pluginId);

private:
    QWeakPointer<Device> m_deviceWeakPtr;
    KdeConnectPlugin *m_sourcePlugin = nullptr;
    PluginId m_sourcePluginId;
};
