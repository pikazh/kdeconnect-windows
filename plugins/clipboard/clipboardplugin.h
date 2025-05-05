/**
 * SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "clipboardlistener.h"
#include "core/plugins/kdeconnectplugin.h"

#include <QClipboard>
#include <QObject>

/**
 * Packet containing just clipboard contents, sent when a device updates its clipboard.
 * <p>
 * The body should look like so:
 * {
 * "content": "password"
 * }
 */
#define PACKET_TYPE_CLIPBOARD QStringLiteral("kdeconnect.clipboard")

/**
 * Packet containing clipboard contents and a timestamp that the contents were last updated, sent
 * on first connection
 * <p>
 * The timestamp is milliseconds since epoch. It can be 0, which indicates that the clipboard
 * update time is currently unknown.
 * <p>
 * The body should look like so:
 * {
 * "timestamp": 542904563213,
 * "content": "password"
 * }
 */
#define PACKET_TYPE_CLIPBOARD_CONNECT QStringLiteral("kdeconnect.clipboard.connect")

class ClipboardPlugin : public KdeConnectPlugin
{
    Q_OBJECT
public:
    explicit ClipboardPlugin(QObject *parent, const QVariantList &args);

public Q_SLOTS:
    void sendClipboard();
    void sendClipboard(const QString &content);

protected:
    void receivePacket(const NetworkPacket &np) override;
    void onPluginEnabled() override;

    void sendConnectPacket();

private Q_SLOTS:
    void onClipboardChanged(const QString &content,
                            ClipboardListener::ClipboardContentType contentType);
    void onConfigChanged();

private:
    bool m_autoShare;
    bool m_sharePasswords;
};
