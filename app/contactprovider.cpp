#include "contactprovider.h"
#include "smshelper.h"

ContactProvider::ContactProvider(Device::Ptr dev, QObject *parent)
    : QObject{parent}
    , m_contactPluginWrapper(new ContactsPluginWrapper(dev, this))
{
    QObject::connect(m_contactPluginWrapper,
                     &ContactsPluginWrapper::localCacheSynchronized,
                     this,
                     &ContactProvider::onPluginContactUpdated);

    QObject::connect(m_contactPluginWrapper,
                     &ContactsPluginWrapper::localCacheRemoved,
                     this,
                     &ContactProvider::onPluginContactDeleted);

    m_contactPluginWrapper->init();

    m_contacts = m_contactPluginWrapper->localCachedContacts();
    for (auto it = m_contacts.begin(); it != m_contacts.end(); ++it) {
        auto &addresses = it.value();
        for (int i = 0; i < addresses.phoneNumbers().count(); ++i) {
            auto &phone = addresses.phoneNumbers().at(i);
            QString canonicalizedNumber = SMSHelper::canonicalizePhoneNumber(phone.number());
            m_phoneAndNameIndexes.insert(canonicalizedNumber, it.key());
        }

        m_phoneAndNameIndexes.insert(addresses.realName(), it.key());
    }
}

KContacts::Addressee ContactProvider::lookupContactByPhoneNumberOrName(const QString &key)
{
    auto it = m_phoneAndNameIndexes.find(key);
    if (it != m_phoneAndNameIndexes.end()) {
        auto &contact = m_contacts[it.value()];
        return contact;
    }

    return KContacts::Addressee();
}

void ContactProvider::onPluginContactUpdated(QHash<QString, KContacts::Addressee> &updatedContacts)
{
    for (auto it = updatedContacts.begin(); it != updatedContacts.end(); ++it) {
        m_contacts[it.key()] = it.value();

        auto &address = it.value();
        for (int i = 0; i < address.phoneNumbers().count(); ++i) {
            auto &phone = address.phoneNumbers().at(i);
            QString canonicalizedNumber = SMSHelper::canonicalizePhoneNumber(phone.number());
            m_phoneAndNameIndexes.insert(canonicalizedNumber, it.key());
        }

        m_phoneAndNameIndexes.insert(address.realName(), it.key());
    }

    Q_EMIT contactUpdated();
}

void ContactProvider::onPluginContactDeleted(const QList<QString> &contactIdsTobeRemoved)
{
    for (auto contactId : contactIdsTobeRemoved) {
        m_phoneAndNameIndexes.removeIf(
            [contactId](QHash<QString, QString>::iterator it) { return it.value() == contactId; });
        m_contacts.remove(contactId);
    }

    Q_EMIT contactUpdated();
}
