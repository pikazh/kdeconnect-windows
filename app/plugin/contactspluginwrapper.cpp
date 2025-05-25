#include "contactspluginwrapper.h"

ContactsPluginWrapper::ContactsPluginWrapper(Device::Ptr devicePtr, QObject *parent)
    : PluginWrapperBase(devicePtr, PluginId::Contacts, parent)
{}

void ContactsPluginWrapper::synchronizeWithRemote()
{
    invokeMethod("synchronizeWithRemote");
}

QHash<QString, KContacts::Addressee> ContactsPluginWrapper::localCachedContacts()
{
    return invokeMethod<QHash<QString, KContacts::Addressee>>("localCachedContacts");
}

void ContactsPluginWrapper::connectPluginSignals(KdeConnectPlugin *plugin)
{
    QObject::connect(plugin,
                     SIGNAL(localCacheSynchronized(QHash<QString, KContacts::Addressee> &)),
                     this,
                     SIGNAL(localCacheSynchronized(QHash<QString, KContacts::Addressee> &)));

    QObject::connect(plugin,
                     SIGNAL(localCacheRemoved(const QList<QString> &)),
                     this,
                     SIGNAL(localCacheRemoved(const QList<QString> &)));
}
