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
    bool isPluginLoaded() const { return m_sourcePlugin != nullptr; }

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

    template<typename T>
    T invokeMethod(const char *methodName)
    {
        T retVal;
        auto plugin = sourcePlugin();
        if (plugin != nullptr) {
            QMetaObject::invokeMethod(plugin,
                                      methodName,
                                      Qt::DirectConnection,
                                      Q_RETURN_ARG(T, retVal));
        }

        return retVal;
    }

    void invokeMethod(const char *methodName)
    {
        auto plugin = sourcePlugin();
        if (plugin != nullptr) {
            QMetaObject::invokeMethod(plugin, methodName, Qt::DirectConnection);
        }
    }

    template<typename T>
    void invokeMethod(const char *methodName, T p1)
    {
        auto plugin = sourcePlugin();
        if (plugin != nullptr) {
            QMetaObject::invokeMethod(plugin, methodName, Qt::DirectConnection, Q_ARG(T, p1));
        }
    }

    template<typename T1, typename T2>
    void invokeMethod(const char *methodName, T1 p1, T2 p2)
    {
        auto plugin = sourcePlugin();
        if (plugin != nullptr) {
            QMetaObject::invokeMethod(plugin,
                                      methodName,
                                      Qt::DirectConnection,
                                      Q_ARG(T1, p1),
                                      Q_ARG(T2, p2));
        }
    }

    template<typename T1, typename T2, typename T3>
    void invokeMethod(const char *methodName, T1 p1, T2 p2, T3 p3)
    {
        auto plugin = sourcePlugin();
        if (plugin != nullptr) {
            QMetaObject::invokeMethod(plugin,
                                      methodName,
                                      Qt::DirectConnection,
                                      Q_ARG(T1, p1),
                                      Q_ARG(T2, p2),
                                      Q_ARG(T3, p3));
        }
    }

    template<typename T1, typename T2, typename T3, typename T4>
    void invokeMethod(const char *methodName, T1 p1, T2 p2, T3 p3, T4 p4)
    {
        auto plugin = sourcePlugin();
        if (plugin != nullptr) {
            QMetaObject::invokeMethod(plugin,
                                      methodName,
                                      Qt::DirectConnection,
                                      Q_ARG(T1, p1),
                                      Q_ARG(T2, p2),
                                      Q_ARG(T3, p3),
                                      Q_ARG(T4, p4));
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    void invokeMethod(const char *methodName, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5)
    {
        auto plugin = sourcePlugin();
        if (plugin != nullptr) {
            QMetaObject::invokeMethod(plugin,
                                      methodName,
                                      Qt::DirectConnection,
                                      Q_ARG(T1, p1),
                                      Q_ARG(T2, p2),
                                      Q_ARG(T3, p3),
                                      Q_ARG(T4, p4),
                                      Q_ARG(T5, p5));
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    void invokeMethod(const char *methodName, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6)
    {
        auto plugin = sourcePlugin();
        if (plugin != nullptr) {
            QMetaObject::invokeMethod(plugin,
                                      methodName,
                                      Qt::DirectConnection,
                                      Q_ARG(T1, p1),
                                      Q_ARG(T2, p2),
                                      Q_ARG(T3, p3),
                                      Q_ARG(T4, p4),
                                      Q_ARG(T5, p5),
                                      Q_ARG(T6, p6));
        }
    }

    virtual void connectPluginSignals(KdeConnectPlugin *plugin) {}

Q_SIGNALS:
    void pluginLoadedChange(bool loaded);

private Q_SLOTS:
    void reloadPlugin();

private:
    KdeConnectPlugin *getPlugin(PluginId pluginId);

private:
    QWeakPointer<Device> m_deviceWeakPtr;
    KdeConnectPlugin *m_sourcePlugin = nullptr;
    PluginId m_sourcePluginId;
};
