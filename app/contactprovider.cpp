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
            if (!phone.isEmpty()) {
                QString canonicalizedNumber = SMSHelper::canonicalizePhoneNumber(phone.number());
                m_phoneNumberIndexes.insert(canonicalizedNumber, it.key());
            }
        }
    }
}

void ContactProvider::synchronize()
{
    m_contactPluginWrapper->synchronize();
}

KContacts::Addressee ContactProvider::lookupContactByPhoneNumber(const QString &key)
{
    auto it = m_phoneNumberIndexes.find(key);
    if (it != m_phoneNumberIndexes.end()) {
        if (it.value() == QStringLiteral("-1")) {
            return KContacts::Addressee();
        }

        auto &contact = m_contacts[it.value()];
        return contact;
    }

    for (auto it2 = m_contacts.begin(); it2 != m_contacts.end(); ++it2) {
        auto &address = it2.value();
        for (int i = 0; i < address.phoneNumbers().count(); ++i) {
            auto &phone = address.phoneNumbers().at(i);
            if (!phone.isEmpty()) {
                QString canonicalizedNumber = SMSHelper::canonicalizePhoneNumber(phone.number());
                if (SMSHelper::isPhoneNumberMatchCanonicalized(canonicalizedNumber, key)) {
                    m_phoneNumberIndexes.insert(key, it2.key());
                    return address;
                }
            }
        }
    }

    m_phoneNumberIndexes.insert(key, QStringLiteral("-1"));

    return KContacts::Addressee();
}

void ContactProvider::onPluginContactUpdated(QHash<QString, KContacts::Addressee> &updatedContacts)
{
    for (auto it = updatedContacts.begin(); it != updatedContacts.end(); ++it) {
        auto &contactId = it.key();
        m_contacts[contactId] = it.value();

        m_phoneNumberIndexes.removeIf([contactId](QHash<QString, QString>::iterator it2) {
            return it2.value() == contactId;
        });

        auto &address = it.value();
        for (int i = 0; i < address.phoneNumbers().count(); ++i) {
            auto &phone = address.phoneNumbers().at(i);
            if (!phone.isEmpty()) {
                QString canonicalizedNumber = SMSHelper::canonicalizePhoneNumber(phone.number());
                m_phoneNumberIndexes.insert(canonicalizedNumber, contactId);
            }
        }
    }

    m_phoneNumberIndexes.removeIf(
        [](QHash<QString, QString>::iterator it2) { return it2.value() == QStringLiteral("-1"); });

    Q_EMIT contactUpdated();
}

void ContactProvider::onPluginContactDeleted(const QList<QString> &contactIdsTobeRemoved)
{
    for (auto contactId : contactIdsTobeRemoved) {
        m_phoneNumberIndexes.removeIf(
            [contactId](QHash<QString, QString>::iterator it) { return it.value() == contactId; });
    }

    Q_EMIT contactUpdated();
}
