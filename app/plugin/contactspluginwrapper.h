#ifndef CONTACTSPLUGINWRAPPER_H
#define CONTACTSPLUGINWRAPPER_H

#include "pluginwrapperbase.h"

#include "addressee.h"

class ContactsPluginWrapper : public PluginWrapperBase
{
    Q_OBJECT
public:
    explicit ContactsPluginWrapper(Device::Ptr devicePtr, QObject *parent = nullptr);
    virtual ~ContactsPluginWrapper() override = default;

public Q_SLOTS:
    void synchronize();
    QHash<QString, KContacts::Addressee> localCachedContacts();

Q_SIGNALS:
    void localCacheSynchronized(QHash<QString, KContacts::Addressee> &);
    void localCacheRemoved(const QList<QString> &contactIdsTobeRemoved);

protected:
    virtual void connectPluginSignals(KdeConnectPlugin *plugin) override;
};

#endif // CONTACTSPLUGINWRAPPER_H
