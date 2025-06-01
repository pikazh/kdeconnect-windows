/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "clipboardplugin.h"
#include "clipboardlistener.h"
#include "core/plugins/pluginfactory.h"
#include "plugin_clipboard_debug.h"

K_PLUGIN_CLASS_WITH_JSON(ClipboardPlugin, "kdeconnect_clipboard.json")

ClipboardPlugin::ClipboardPlugin(QObject *parent, const QVariantList &args)
    : KdeConnectPlugin(parent, args)
{
    connect(ClipboardListener::instance(),
            &ClipboardListener::clipboardChanged,
            this,
            &ClipboardPlugin::onClipboardChanged);
    connect(config().get(),
            &KdeConnectPluginConfig::configChanged,
            this,
            &ClipboardPlugin::reloadConfig);

    reloadConfig();
}

void ClipboardPlugin::onPluginEnabled()
{
    sendConnectPacket();
}

void ClipboardPlugin::onClipboardChanged(const QString &content,
                                         ClipboardListener::ClipboardContentType contentType)
{
    if (!m_autoShare) {
        return;
    }

    if (contentType == ClipboardListener::ClipboardContentTypePassword) {
        if (!m_sharePasswords) {
            return;
        }
    }

    sendClipboard(content);
}

void ClipboardPlugin::reloadConfig()
{
    m_autoShare = config()->getBool(QStringLiteral("autoShare"), false);
    m_sharePasswords = config()->getBool(QStringLiteral("sendPassword"), false);
}

void ClipboardPlugin::sendClipboard()
{
    QString content = ClipboardListener::instance()->currentContent();
    sendClipboard(content);
}

void ClipboardPlugin::sendClipboard(const QString &content)
{
    NetworkPacket np(PACKET_TYPE_CLIPBOARD, {{QStringLiteral("content"), content}});
    sendPacket(np);
}

void ClipboardPlugin::sendConnectPacket()
{
    if (!m_autoShare
        || (!m_sharePasswords
            && ClipboardListener::instance()->currentContentType()
                   == ClipboardListener::ClipboardContentTypePassword)) {
        // avoid sharing clipboard if the content is no need to share
        return;
    }

    NetworkPacket np(PACKET_TYPE_CLIPBOARD_CONNECT,
                     {{QStringLiteral("content"), ClipboardListener::instance()->currentContent()},
                      {QStringLiteral("timestamp"), ClipboardListener::instance()->updateTimestamp()}});
    sendPacket(np);
}

void ClipboardPlugin::receivePacket(const NetworkPacket &np)
{
    if (np.type() == PACKET_TYPE_CLIPBOARD) {
        QString content = np.get<QString>(QStringLiteral("content"));
        ClipboardListener::instance()->setText(content);
    } else if (np.type() == PACKET_TYPE_CLIPBOARD_CONNECT) {
        qint64 packetTime = np.get<qint64>(QStringLiteral("timestamp"));
        // If the packetTime is 0, it means the timestamp is unknown (so do nothing).
        if (packetTime == 0 || packetTime < ClipboardListener::instance()->updateTimestamp()) {
            qCWarning(KDECONNECT_PLUGIN_CLIPBOARD) << "Ignoring clipboard without timestamp";
            return;
        }

        if (np.has(QStringLiteral("content"))) {
            QString content = np.get<QString>(QStringLiteral("content"));
            ClipboardListener::instance()->setText(content);
        }
    }
}

#include "clipboardplugin.moc"
