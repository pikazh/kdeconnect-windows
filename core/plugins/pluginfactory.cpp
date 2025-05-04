#include "pluginfactory.h"

#include <QPluginLoader>
#include <QPair>
#include <vector>

class PluginFactoryPrivate
{
public:
    using PluginWithMetadata = QPair<const QMetaObject *, PluginFactory::CreateInstanceWithMetaDataFunction>;
    PluginMetaData metaData;
    std::vector<PluginWithMetadata> createInstanceWithMetaDataHash;
};

PluginFactory::PluginFactory(QObject *parent)
    : QObject(parent)
    , d(new PluginFactoryPrivate)
{

}

PluginFactory::~PluginFactory()
{

}

PluginFactory *PluginFactory::loadFactory(const PluginMetaData &data)
{
    if(!data.filePath().isEmpty())
    {
        QPluginLoader loader(data.filePath());
        QObject *obj = loader.instance();
        if(obj != nullptr)
        {
            PluginFactory *factory = qobject_cast<PluginFactory*>(obj);
            if(factory != nullptr)
            {
                factory->setMetaData(data);
            }

            return factory;
        }
    }

    return nullptr;
}

void PluginFactory::registerPlugin(const QMetaObject *metaObject, CreateInstanceWithMetaDataFunction instanceFunction)
{
    d->createInstanceWithMetaDataHash.push_back({metaObject, instanceFunction});
}

QObject *PluginFactory::create(const char *iface, QObject *parent, const QVariantList &args)
{
    for (const PluginFactoryPrivate::PluginWithMetadata &plugin : d->createInstanceWithMetaDataHash) {
        for (const QMetaObject *current = plugin.first; current; current = current->superClass()) {
            if (0 == qstrcmp(iface, current->className())) {
                return plugin.second(parent, d->metaData, args);
            }
        }
    }

    return nullptr;
}

PluginMetaData PluginFactory::metaData() const
{
    return d->metaData;
}

void PluginFactory::setMetaData(const PluginMetaData &metaData)
{
    d->metaData = metaData;
}
