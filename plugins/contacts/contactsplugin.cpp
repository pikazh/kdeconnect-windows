/**
 * SPDX-FileCopyrightText: 2018 Simon Redman <simon@ergotech.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "contactsplugin.h"
#include "contactsdb.h"
#include "plugin_contacts_debug.h"

#include "core/kdeconnectconfig.h"
#include "core/plugins/pluginfactory.h"

#include "vcardconverter.h"

#include <QDir>
#include <QString>

K_PLUGIN_CLASS_WITH_JSON(ContactsPlugin, "kdeconnect_contacts.json")

ContactsPlugin::ContactsPlugin(QObject *parent, const QVariantList &args)
    : KdeConnectPlugin(parent, args)
    , m_contactsDB(new ContactsDB(this))
{
    QString dbPath = KdeConnectConfig::instance().deviceDataDir(device()->id()).absolutePath();
    QDir().mkpath(dbPath);
    dbPath = dbPath + QDir::separator() + QStringLiteral("contacts");
    QString connectionName = device()->id() + QStringLiteral("/contacts");
    m_contactsDB->init(connectionName, dbPath);

    synchronize();
}

void ContactsPlugin::receivePacket(const NetworkPacket &np)
{
    if (np.type() == PACKAGE_TYPE_CONTACTS_RESPONSE_UIDS_TIMESTAMPS) {
        handleResponseUIDsTimestamps(np);
    } else if (np.type() == PACKET_TYPE_CONTACTS_RESPONSE_VCARDS) {
        handleResponseVCards(np);
    }
}

void ContactsPlugin::synchronize()
{
    sendRequest(PACKET_TYPE_CONTACTS_REQUEST_ALL_UIDS_TIMESTAMP);
}

QHash<QString, KContacts::Addressee> ContactsPlugin::localCachedContacts()
{
    QHash<QString, KContacts::Addressee> rets;
    auto dataList = m_contactsDB->allData();
    for (std::tuple<QString, qint64, QByteArray> &data : dataList) {
        KContacts::VCardConverter converter;
        KContacts::Addressee::List list = converter.parseVCards(std::get<2>(data));
        if (!list.isEmpty()) {
            rets.insert(std::get<0>(data), list.at(0));
        }
    }

    return rets;
}

bool ContactsPlugin::handleResponseUIDsTimestamps(const NetworkPacket &np)
{
    if (!np.has(QStringLiteral("uids"))) {
        qWarning(KDECONNECT_PLUGIN_CONTACTS) << "handleResponseUIDsTimestamps:"
                                             << "Malformed packet does not have uids key";
        return false;
    }

    QStringList uIDs = np.get<QStringList>(QStringLiteral("uids"));

    // Check local storage for the contacts:
    //  If the contact is not found in local storage, request its vcard be sent
    //  If the contact is in local storage but not reported, delete it
    //  If the contact is in local storage, compare its timestamp. If different, request the contact

    auto uidAndTimeStampsFromDB = m_contactsDB->allUidAndTimeStamps();

    for (auto it = uIDs.begin(); it != uIDs.end();) {
        auto it2 = uidAndTimeStampsFromDB.find(*it);
        if (it2 != uidAndTimeStampsFromDB.end()) {
            qint64 localTimeStamp = it2.value();
            uidAndTimeStampsFromDB.erase(it2);
            qint64 remoteTimestamp = np.get<qint64>(*it);
            if (remoteTimestamp == localTimeStamp) {
                it = uIDs.erase(it);
                continue;
            }
        }

        ++it;
    }

    if (!uIDs.isEmpty()) {
        sendRequestWithIDs(PACKET_TYPE_CONTACTS_REQUEST_VCARDS_BY_UIDS, uIDs);
        qInfo(KDECONNECT_PLUGIN_CONTACTS) << "request vcard content by id:" << uIDs;
    }

    if (!uidAndTimeStampsFromDB.isEmpty()) {
        m_contactsDB->deleteRecords(uidAndTimeStampsFromDB.keys());
        qInfo(KDECONNECT_PLUGIN_CONTACTS)
            << "delete contacts not reported:" << uidAndTimeStampsFromDB.keys();

        Q_EMIT localCacheRemoved(uidAndTimeStampsFromDB.keys());
    }

    return true;
}

bool ContactsPlugin::handleResponseVCards(const NetworkPacket &np)
{
    if (!np.has(QStringLiteral("uids"))) {
        qCDebug(KDECONNECT_PLUGIN_CONTACTS) << "handleResponseVCards:"
                                            << "Malformed packet does not have uids key";
        return false;
    }

    const QStringList &uIDs = np.get<QStringList>(QStringLiteral("uids"));

    QHash<QString, KContacts::Addressee> updatedContacts;
    // Loop over all IDs, extract the VCard from the packet and write the file
    for (const auto &ID : uIDs) {
        QByteArray buf = np.get<QByteArray>(ID);
        KContacts::VCardConverter converter;
        KContacts::Addressee::List list = converter.parseVCards(buf);
        if (list.isEmpty()) {
            qCritical(KDECONNECT_PLUGIN_CONTACTS) << "parse vcard failed.";
        } else {
            QString timeStampStr = list[0].custom(QStringLiteral("KDECONNECT"),
                                                  QStringLiteral("TIMESTAMP"));
            qint64 timeStamp = timeStampStr.toLongLong();
            if (m_contactsDB->insertOrUpdateRecord(ID, timeStamp, buf)) {
                qCDebug(KDECONNECT_PLUGIN_CONTACTS) << "record with uid" << ID << "updated";
            }

            updatedContacts[ID] = list[0];
        }
    }

    if (!updatedContacts.isEmpty()) {
        Q_EMIT localCacheSynchronized(updatedContacts);
    }

    return true;
}

bool ContactsPlugin::sendRequest(const QString &packetType)
{
    NetworkPacket np(packetType);
    bool success = sendPacket(np);
    //qCDebug(KDECONNECT_PLUGIN_CONTACTS) << "sendRequest: Sending " << packetType << success;

    return success;
}

bool ContactsPlugin::sendRequestWithIDs(const QString &packetType, const QStringList &uIDs)
{
    NetworkPacket np(packetType);

    np.set<QStringList>(QStringLiteral("uids"), uIDs);
    bool success = sendPacket(np);
    return success;
}

#include "contactsplugin.moc"
