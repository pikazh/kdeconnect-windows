/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 * SPDX-FileCopyrightText: 2018 Simon Redman <simon@ergotech.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "smsplugin.h"
#include "plugin_sms_debug.h"

#include "core/device.h"
#include "core/kdeconnectconfig.h"
#include "core/plugins/pluginfactory.h"

#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>

K_PLUGIN_CLASS_WITH_JSON(SmsPlugin, "kdeconnect_sms.json")

SmsPlugin::SmsPlugin(QObject *parent, const QVariantList &args)
    : KdeConnectPlugin(parent, args)
{
}

void SmsPlugin::receivePacket(const NetworkPacket &np)
{
    if (np.type() == PACKET_TYPE_SMS_MESSAGES) {
        handleBatchMessages(np);
    }

    if (np.type() == PACKET_TYPE_SMS_ATTACHMENT_FILE && np.hasPayload()) {
        handleSmsAttachmentFile(np);
    }
}

bool SmsPlugin::sendSms(const QList<QString> &addresses,
                        const QString &textMessage,
                        const QList<QString> &attachmentUrls,
                        const qint64 subID)
{
    QVariantList addressMapList;
    for (const QString &address : addresses) {
        if (!address.isEmpty()) {
            QVariantMap addressMap({{QStringLiteral("address"), address}});
            addressMapList.append(addressMap);
        }
    }

    if (addressMapList.isEmpty()) {
        return false;
    }

    QVariantMap packetMap({{QStringLiteral("version"), SMS_REQUEST_PACKET_VERSION},
                           {QStringLiteral("addresses"), addressMapList}});

    // If there is any text message add it to the network packet
    if (!textMessage.trimmed().isEmpty()) {
        packetMap[QStringLiteral("messageBody")] = textMessage.trimmed();
    } else {
        return false;
    }

    if (subID != -1) {
        packetMap[QStringLiteral("subID")] = subID;
    }

    QVariantList attachmentMapList;
    for (const QString &attachmentUrl : attachmentUrls) {
        const Attachment attachment = createAttachmentFromUrl(attachmentUrl);
        if (attachment.isValid()) {
            QVariantMap attachmentMap(
                {{QStringLiteral("fileName"), attachment.uniqueIdentifier()},
                 {QStringLiteral("base64EncodedFile"), attachment.base64EncodedFile()},
                 {QStringLiteral("mimeType"), attachment.mimeType()}});
            attachmentMapList.append(attachmentMap);
        }
    }

    // If there is any attachment add it to the network packet
    if (!attachmentMapList.isEmpty()) {
        packetMap[QStringLiteral("attachments")] = attachmentMapList;
    }

    NetworkPacket np(PACKET_TYPE_SMS_REQUEST, packetMap);
    qDebug(KDECONNECT_PLUGIN_SMS) << "Dispatching SMS send request to remote";
    return sendPacket(np);
}

void SmsPlugin::requestAllConversations()
{
    NetworkPacket np(PACKET_TYPE_SMS_REQUEST_CONVERSATIONS);
    sendPacket(np);
}

void SmsPlugin::requestConversation(const qint64 conversationID, const qint64 rangeStartTimestamp, const qint64 numberToRequest) const
{
    NetworkPacket np(PACKET_TYPE_SMS_REQUEST_CONVERSATION);
    np.set(QStringLiteral("threadID"), conversationID);
    np.set(QStringLiteral("rangeStartTimestamp"), rangeStartTimestamp);
    np.set(QStringLiteral("numberToRequest"), numberToRequest);

    sendPacket(np);
}

void SmsPlugin::requestAttachment(const qint64 &partID, const QString &uniqueIdentifier)
{
    const QVariantMap packetMap({{QStringLiteral("part_id"), partID}, {QStringLiteral("unique_identifier"), uniqueIdentifier}});

    NetworkPacket np(PACKET_TYPE_SMS_REQUEST_ATTACHMENT, packetMap);

    sendPacket(np);
}

bool SmsPlugin::handleBatchMessages(const NetworkPacket &np)
{
    const auto messages = np.get<QVariantList>(QStringLiteral("messages"));
    QList<ConversationMessage> messagesList;
    messagesList.reserve(messages.count());

    for (const QVariant &body : messages) {
        ConversationMessage message(body.toMap());
        messagesList.append(message);
    }

    if (messages.count() > 0) {
        Q_EMIT messagesReceived(messagesList);
    }

    return true;
}

bool SmsPlugin::handleSmsAttachmentFile(const NetworkPacket &np)
{
    const QString fileName = np.get<QString>(QStringLiteral("filename"));
    const auto fileSize = np.payloadSize();
    const QVariantMap payloadInfo = np.payloadTransferInfo();
    const QString host = qvariant_cast<QString>(payloadInfo[QStringLiteral("host")]);
    const quint16 port = qvariant_cast<quint16>(payloadInfo[QStringLiteral("port")]);
    Q_ASSERT(!fileName.isEmpty() && fileSize > 0 && !host.isEmpty() && port > 0);
    if (!fileName.isEmpty() && fileSize > 0 && !host.isEmpty() && port > 0) {
        Q_EMIT attachmentDownloadInfoReceived(fileName, fileSize, host, port);
        return true;
    }

    return false;
}

Attachment SmsPlugin::createAttachmentFromUrl(const QString &url)
{
    QFile file(url);
    file.open(QIODevice::ReadOnly);

    if (!file.exists()) {
        return Attachment();
    }

    QFileInfo fileInfo(file);
    QString fileName(fileInfo.fileName());

    QString base64EncodedFile = QString::fromLatin1(file.readAll().toBase64());
    file.close();

    QMimeDatabase mimeDatabase;
    QString mimeType = mimeDatabase.mimeTypeForFile(url).name();

    Attachment attachment(-1, mimeType, base64EncodedFile, fileName);
    return attachment;
}

#include "smsplugin.moc"
