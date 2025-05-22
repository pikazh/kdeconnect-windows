/**
 * SPDX-FileCopyrightText: 2018 Simon Redman <simon@ergotech.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "addressee.h"
#include "core/plugins/kdeconnectplugin.h"

#include <QHash>

/**
 * Used to request the device send the unique ID and last-changed timestamp of every contact
 */
#define PACKET_TYPE_CONTACTS_REQUEST_ALL_UIDS_TIMESTAMP QStringLiteral("kdeconnect.contacts.request_all_uids_timestamps")

/**
 * Used to request the vcards for the contacts corresponding to a list of UIDs
 *
 * It shall contain the key "uids", which will have a list of uIDs (long int, as string)
 */
#define PACKET_TYPE_CONTACTS_REQUEST_VCARDS_BY_UIDS QStringLiteral("kdeconnect.contacts.request_vcards_by_uid")

/**
 * Response indicating the package contains a list of all contact uIDs and last-changed timestamps
 *
 * It shall contain the key "uids", which will mark a list of uIDs (long int, as string)
 * then, for each UID, there shall be a field with the key of that UID and the value of the timestamp (int, as string)
 *
 * For example:
 * ( 'uids' : ['1', '3', '15'],
 *  '1'  : '973486597',
 *  '3'  : '973485443',
 *  '15' : '973492390' )
 *
 * The returned IDs can be used in future requests for more information about the contact
 */
#define PACKAGE_TYPE_CONTACTS_RESPONSE_UIDS_TIMESTAMPS QStringLiteral("kdeconnect.contacts.response_uids_timestamps")

/**
 * Response indicating the package contains a list of contact vcards
 *
 * It shall contain the key "uids", which will mark a list of uIDs (long int, as string)
 * then, for each UID, there shall be a field with the key of that UID and the value of the remote's vcard for that contact
 *
 * For example:
 * ( 'uids' : ['1', '3', '15'],
 *  '1'  : 'BEGIN:VCARD\n....\nEND:VCARD',
 *  '3'  : 'BEGIN:VCARD\n....\nEND:VCARD',
 *  '15' : 'BEGIN:VCARD\n....\nEND:VCARD' )
 */
#define PACKET_TYPE_CONTACTS_RESPONSE_VCARDS QStringLiteral("kdeconnect.contacts.response_vcards")

class ContactsDB;

class ContactsPlugin : public KdeConnectPlugin
{
    Q_OBJECT
public:
    explicit ContactsPlugin(QObject *parent, const QVariantList &args);

protected:
    void receivePacket(const NetworkPacket &np) override;

public Q_SLOTS:

    /**
     * 	Query the remote device for all its uIDs and last-changed timestamps, then:
     *      Delete any contacts which are known locally but not reported by the remote
     *      Update any contacts which are known locally but have an older timestamp
     *      Add any contacts which are not known locally but are reported by the remote
     */
    Q_SCRIPTABLE void synchronizeWithRemote();

    QHash<QString, KContacts::Addressee> localCachedContacts();

Q_SIGNALS:
    /**
     * Emitted to indicate that we have locally cached all remote contacts
     *
     * @param newContact The just-synchronized contact 
     */
    Q_SCRIPTABLE void localCacheSynchronized(const QString &newContactID,
                                             const KContacts::Addressee &address);

    void localCacheRemoved(const QList<QString> &contactIdsTobeRemoved);

protected:
    /**
     *	Handle a packet of type PACKAGE_TYPE_CONTACTS_RESPONSE_UIDS_TIMESTAMPS
     *
     *  For every uID in the reply:
     *      Delete any from local storage if it does not appear in the reply
     *      Compare the modified timestamp for each in the reply and update any which should have changed
     *      Request the details any IDs which were not locally cached
     */
    bool handleResponseUIDsTimestamps(const NetworkPacket &);

    /**
     *  Handle a packet of type PACKET_TYPE_CONTACTS_RESPONSE_VCARDS
     */
    bool handleResponseVCards(const NetworkPacket &);

    /**
     * Send a request-type packet which contains no body
     *
     * @return True if the send was successful, false otherwise
     */
    bool sendRequest(const QString &packetType);

    /**
     * Send a request-type packet which has a body with the key 'uids' and the value the list of
     * specified uIDs
     *
     * @param packageType Type of package to send
     * @param uIDs List of uIDs to request
     * @return True if the send was successful, false otherwise
     */
    bool sendRequestWithIDs(const QString &packetType, const QStringList &uIDs);

private:
    ContactsDB *m_contactsDB;
};
