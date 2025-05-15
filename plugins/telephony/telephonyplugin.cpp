/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 * SPDX-FileCopyrightText: 2018 Simon Redman <simon@ergotech.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "telephonyplugin.h"
#include "core/plugins/pluginfactory.h"
#include "plugin_telephony_debug.h"

#include <QPixmap>

K_PLUGIN_CLASS_WITH_JSON(TelephonyPlugin, "kdeconnect_telephony.json")

void TelephonyPlugin::handlePackage(const NetworkPacket &np)
{
    if (np.get<bool>(QStringLiteral("isCancel"))) {
        if (m_currentCallNotification) {
            m_currentCallNotification->close();
            m_currentCallNotification.clear();
        }
        return;
    }

    const QString event = np.get<QString>(QStringLiteral("event"));
    const QString phoneNumber = np.get<QString>(QStringLiteral("phoneNumber"), tr("unknown number"));
    const QString contactName = np.get<QString>(QStringLiteral("contactName"), phoneNumber);
    const QByteArray phoneThumbnail = QByteArray::fromBase64(np.get<QByteArray>(QStringLiteral("phoneThumbnail"), ""));

    QString content, type, icon;

    if (event == QLatin1String("ringing")) {
        type = QStringLiteral("callReceived");
        icon = QStringLiteral("call-start");
        content = tr("Incoming call from ") + contactName;
    } else if (event == QLatin1String("missedCall")) {
        type = QStringLiteral("missedCall");
        icon = QStringLiteral("call-start");
        content = tr("Missed call from ") + contactName;
    } else if (event == QLatin1String("talking")) {
        type = QStringLiteral("talking");
    }

    Q_EMIT callReceived(type, phoneNumber, contactName);

    if (event == QLatin1String("talking")) {
        if (m_currentCallNotification) {
            m_currentCallNotification->close();
            m_currentCallNotification.clear();
        }
        return;
    }

    qCDebug(KDECONNECT_PLUGIN_TELEPHONY) << "Creating notification with type:" << type;

    if (!m_currentCallNotification) {
        m_currentCallNotification = new Notification();
    }

    if (!phoneThumbnail.isEmpty()) {
        QPixmap photo;
        photo.loadFromData(phoneThumbnail, "JPEG");
        m_currentCallNotification->setPixmap(photo);
    } else {
        m_currentCallNotification->setIconName(icon);
    }

    m_currentCallNotification->setTitle(device()->name());
    m_currentCallNotification->setText(content);

    m_currentCallNotification->clearActions();
    if (event == QLatin1String("ringing")) {
        NotificationAction *muteAction = m_currentCallNotification->addAction(tr("Mute Call"));
        QObject::connect(muteAction,
                         &NotificationAction::activated,
                         this,
                         &TelephonyPlugin::sendMutePacket);
    }

    m_currentCallNotification->sendNotify();
}

void TelephonyPlugin::receivePacket(const NetworkPacket &np)
{
    // ignore old style sms packet
    if (np.get<QString>(QStringLiteral("event")) != QLatin1String("sms")) {
        handlePackage(np);
    }
}

void TelephonyPlugin::sendMutePacket()
{
    NetworkPacket packet(PACKET_TYPE_TELEPHONY_REQUEST_MUTE, {{QStringLiteral("action"), QStringLiteral("mute")}});
    sendPacket(packet);
}

#include "telephonyplugin.moc"
