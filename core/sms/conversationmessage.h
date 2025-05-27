/**
 * SPDX-FileCopyrightText: 2018 Simon Redman <simon@ergotech.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLUGINS_TELEPHONY_CONVERSATIONMESSAGE_H_
#define PLUGINS_TELEPHONY_CONVERSATIONMESSAGE_H_

#include "kdeconnectcore_export.h"

#include <QMetaType>
#include <QObject>

#include <QVariantMap>

class ConversationAddress;
class Attachment;

class KDECONNECTCORE_EXPORT ConversationMessage
{
public:
    // TYPE field values from Android
    enum Types {
        MessageTypeAll = 0,
        MessageTypeInbox = 1,
        MessageTypeSent = 2,
        MessageTypeDraft = 3,
        MessageTypeOutbox = 4,
        MessageTypeFailed = 5,
        MessageTypeQueued = 6,
    };

    /**
     * Values describing the possible type of events contained in a message
     * A message's eventField is constructed as a bitwise-OR of events
     * Any events which are unsupported should be ignored
     */
    enum Events {
        EventTextMessage = 0x1, // This message has a body field which contains pure, human-readable text
        EventMultiTarget = 0x2, // This is a multitarget (group) message which has an "addresses" field which is a list of participants in the group
    };

    /**
     * Build a new message from a keyword argument dictionary
     *
     * @param args mapping of field names to values as might be contained in a network packet containing a message
     */
    ConversationMessage(const QVariantMap &args = QVariantMap());

    ConversationMessage(const qint32 &eventField,
                        const QString &body,
                        const QList<ConversationAddress> &addresses,
                        const qint64 &date,
                        const qint32 &type,
                        const qint32 &read,
                        const qint64 &threadID,
                        const qint32 &uID,
                        const qint64 &subID,
                        const QList<Attachment> &attachments);

    qint32 eventField() const { return m_eventField; }
    QString body() const
    {
        return m_body;
    }
    QList<ConversationAddress> addresses() const
    {
        return m_addresses;
    }
    qint64 date() const
    {
        return m_date;
    }
    qint32 type() const
    {
        return m_type;
    }
    qint32 read() const
    {
        return m_read;
    }
    qint64 threadID() const
    {
        return m_threadID;
    }
    qint32 uID() const
    {
        return m_uID;
    }
    qint64 subID() const
    {
        return m_subID;
    }
    QList<Attachment> attachments() const
    {
        return m_attachments;
    }

    bool containsTextBody() const
    {
        return (eventField() & ConversationMessage::EventTextMessage);
    }
    bool isMultitarget() const
    {
        return (eventField() & ConversationMessage::EventMultiTarget);
    }

    bool isIncoming() const
    {
        return type() == MessageTypeInbox;
    }
    bool isOutgoing() const;
    bool containsAttachment() const
    {
        return !attachments().isEmpty();
    }

    /**
     * Return the address of the other party of a single-target conversation
     * Calling this method with a multi-target conversation is ill-defined
     */
    QString getOtherPartyAddress() const;

protected:
    /**
     * Bitwise OR of event flags
     * Unsupported flags shall cause the message to be ignored
     */
    qint32 m_eventField;

    /**
     * Body of the message
     */
    QString m_body;

    /**
     * List of all addresses involved in this conversation
     * An address is most likely a phone number, but may be something else like an email address
     */
    QList<ConversationAddress> m_addresses;

    /**
     * Date stamp (Unix epoch millis) associated with the message
     */
    qint64 m_date;

    /**
     * Type of the message. See the message.type enum
     */
    qint32 m_type;

    /**
     * Whether we have a read report for this message
     */
    qint32 m_read;

    /**
     * Tag which binds individual messages into a thread
     */
    qint64 m_threadID;

    /**
     * Value which uniquely identifies a message
     */
    qint32 m_uID;

    /**
     * Value which determines SIM id (optional)
     */
    qint64 m_subID;

    /**
     * Contains attachment related info of a MMS message (optional)
     */
    QList<Attachment> m_attachments;
};

class KDECONNECTCORE_EXPORT ConversationAddress
{
public:
    ConversationAddress(QString address = QString());

    QString address() const
    {
        return m_address;
    }

private:
    QString m_address;
};

class KDECONNECTCORE_EXPORT Attachment
{
public:
    Attachment()
    {
    }
    Attachment(qint64 partID, QString mimeType, QString base64EncodedFile, QString uniqueIdentifier);

    qint64 partID() const
    {
        return m_partID;
    }
    QString mimeType() const
    {
        return m_mimeType;
    }
    QString base64EncodedFile() const
    {
        return m_base64EncodedFile;
    }
    QString uniqueIdentifier() const
    {
        return m_uniqueIdentifier;
    }
    bool isValid() const
    {
        return !(m_uniqueIdentifier.isEmpty() || m_mimeType.isEmpty() || m_partID < 0);
    }

private:
    qint64 m_partID; // Part ID of the attachment of the message
    QString m_mimeType; // Type of attachment (image, video, audio etc.)
    QString m_base64EncodedFile; // Base64 encoded string of a file
    QString m_uniqueIdentifier; // unique name of the attachment
};

Q_DECLARE_METATYPE(ConversationMessage)
Q_DECLARE_METATYPE(ConversationAddress)
Q_DECLARE_METATYPE(Attachment)

#endif /* PLUGINS_TELEPHONY_CONVERSATIONMESSAGE_H_ */
