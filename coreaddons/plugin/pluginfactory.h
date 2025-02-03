#pragma once

#include "coreaddons_export.h"
#include "pluginmetadata.h"

#include <QObject>
#include <QVariant>
#include <memory>

#define KPluginFactory_iid "org.kde.PluginFactory"

// Internal macro that generated the PluginFactory subclass
#define __K_PLUGIN_FACTORY_DEFINITION(name, pluginRegistrations, ...)          \
class name : public PluginFactory                                              \
{                                                                              \
    Q_OBJECT                                                                   \
    Q_INTERFACES(PluginFactory)                                                \
    Q_PLUGIN_METADATA(__VA_ARGS__)                                             \
public:                                                                        \
    explicit name(QObject *parent = nullptr)                                   \
        : PluginFactory(parent)                                                \
    {                                                                          \
         pluginRegistrations                                                   \
    }                                                                          \
    ~name() = default;                                                         \
};

#define K_PLUGIN_FACTORY_WITH_JSON(name, jsonFile, pluginRegistrations)        \
__K_PLUGIN_FACTORY_DEFINITION(name, pluginRegistrations, IID KPluginFactory_iid FILE jsonFile)

#define K_PLUGIN_CLASS_WITH_JSON(classname, jsonFile) K_PLUGIN_FACTORY_WITH_JSON(classname##Factory, jsonFile, registerPlugin<classname>();)

class PluginFactoryPrivate;
class COREADDONS_EXPORT PluginFactory: public QObject
{
    Q_OBJECT
public:
    explicit PluginFactory(QObject* parent);
    virtual ~PluginFactory();

    static PluginFactory* loadFactory(const PluginMetaData &data);

    template<typename T>
    static T* instantiatePlugin(const PluginMetaData &data, QObject *parent = nullptr, const QVariantList &args = {})
    {
        PluginFactory *factory = loadFactory(data);
        if(factory != nullptr)
        {
            T *instance = factory->create<T>(parent, args);
            return instance;
        }

        return nullptr;
    }

protected:
    friend class PluginFactoryPrivate;

    template<class T>
    void registerPlugin()
    {
        registerPlugin(&T::staticMetaObject, &createInstance<T>);
    }

    using CreateInstanceWithMetaDataFunction = QObject *(*)(QObject *, const PluginMetaData &, const QVariantList &);

    void registerPlugin(const QMetaObject *metaObject, CreateInstanceWithMetaDataFunction instanceFunction);

    template<class impl>
    static QObject *createInstance(QObject *parent, const PluginMetaData & /*metaData*/, const QVariantList &args)
    {
        return new impl(parent, args);
    }

    template<typename T>
    T* create(QObject *parent, const QVariantList &args)
    {
        QObject *o = create(T::staticMetaObject.className(), parent, args);

        T *t = qobject_cast<T *>(o);
        if (t == nullptr) {
            delete o;
        }
        return t;
    }

    virtual QObject *create(const char *iface, QObject *parent, const QVariantList &args);

    PluginMetaData metaData() const;
    void setMetaData(const PluginMetaData &metaData);
private:
    std::unique_ptr<PluginFactoryPrivate> const d;
};

Q_DECLARE_INTERFACE(PluginFactory, KPluginFactory_iid)
