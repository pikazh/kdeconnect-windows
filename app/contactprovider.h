#ifndef CONTACTPROVIDER_H
#define CONTACTPROVIDER_H

#include "core/device.h"
#include "plugin/contactspluginwrapper.h"

#include <QHash>
#include <QObject>

class ContactProvider : public QObject
{
    Q_OBJECT
public:
    explicit ContactProvider(Device::Ptr dev, QObject *parent = nullptr);

    KContacts::Addressee lookupContactByPhoneNumberOrName(const QString &key);

protected Q_SLOTS:
    void onPluginContactUpdated(QHash<QString, KContacts::Addressee> &updatedContacts);
    void onPluginContactDeleted(const QList<QString> &contactIdsTobeRemoved);

Q_SIGNALS:
    void contactUpdated();

private:
    ContactsPluginWrapper *m_contactPluginWrapper = nullptr;

    QHash<QString, KContacts::Addressee> m_contacts;
    QHash<QString, QString> m_phoneAndNameIndexes;
};

#endif // CONTACTPROVIDER_H
